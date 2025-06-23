#include "sensorReader.h"

#include "config.h"
#include "logger.h"

void SensorReader::begin() {
    analogSetAttenuation(ADC_11db);  // up to ≈3.6V
    DEBUG_INFO(F("SensorReader ready"));
}

void SensorReader::update() {
    data_.voltageSocket = readVoltage(PIN_VDIV1);    // main voltage divider
    data_.voltageContacts = readVoltage(PIN_VDIV2);  // backup voltage divider

    // Determine active voltage divider and switch appropriate relay
    if (data_.voltageContacts > 1.0f) {
        data_.voltage = data_.voltageContacts;
        digitalWrite(PIN_RELAY1, HIGH);
        digitalWrite(PIN_RELAY2, LOW);
    } else if (data_.voltageSocket > 1.0f) {
        data_.voltage = data_.voltageSocket;
        digitalWrite(PIN_RELAY1, LOW);
        digitalWrite(PIN_RELAY2, HIGH);
    } else {
        // no voltage detected
        digitalWrite(PIN_RELAY1, LOW);
        digitalWrite(PIN_RELAY2, LOW);
    }

    data_.current = readCurrent(PIN_CURRENT);
    data_.tempAmbient = readNTC(PIN_TEMP_AMBIENT);
    // data_.tempDC = readNTC(PIN_TEMP_DC);
    data_.tempNeg = readNTC(PIN_TEMP_NEG);
    data_.tempPos = readNTC(PIN_TEMP_POS);
    data_.millisTs = millis();

    DEBUG_F("Voltage Socket: %f", data_.voltageSocket);
    DEBUG_F("Voltage Contacts: %f", data_.voltageContacts);
    DEBUG_F("Current: %f", data_.current);
    DEBUG_F("TempAmbient: %f", data_.tempAmbient);
    // DEBUG_F("TempDC: %f", data_.tempDC);
    DEBUG_F("TempNeg: %f", data_.tempNeg);
    DEBUG_F("TempPos: %f", data_.tempPos);
    DEBUG_F("MillisTs: %d", data_.millisTs);
}

float SensorReader::readVoltage(uint8_t pin) {
    uint16_t raw = analogRead(pin);
    float vin = (raw / ADC_FULL_SCALE) * ADC_REF_V / VDIV_RATIO;
    return vin;
}

// float SensorReader::readCurrent(uint8_t pin) {
//     uint16_t raw = analogRead(pin);
//     float vout = (raw / ADC_FULL_SCALE) * ADC_REF_V;
//     float amp = (vout - CURRENT_VREF) / CURRENT_KA;
//     return amp;
// }

float SensorReader::readCurrent(uint8_t pin) {
    const uint8_t SAMPLES = 16;  // количество усредняемых выборок
    uint32_t sum = 0;
    for (uint8_t i = 0; i < SAMPLES; ++i) {
        sum += analogRead(pin);
    }
    const float adcMean = static_cast<float>(sum) / SAMPLES;

    const float voltage = (adcMean / 4095.0f) * VREF;

    const float current = (voltage - V_OFFSET) / SENSITIVITY;
    return current;
}

float SensorReader::readNTC(uint8_t pin) {
    uint16_t raw = analogRead(pin);
    float v = (raw / ADC_FULL_SCALE) * ADC_REF_V;
    float r = (ADC_REF_V * 10000.0f / v) - 10000.0f;  // 10k divider with NTC
    // GyverNTC approximation
    const float beta = 3800.0f;
    const float t0K = 298.15f;  // 25°C
    const float r0 = 10000.0f;
    float tempK = 1.0f / (1.0f / t0K + 1.0f / beta * log(r / r0));
    return tempK - 273.15f;
}
