#include "chargerFSM.h"

#include <ArduinoJson.h>

#include "charge_state_utils.h"
#include "config.h"
#include "logger.h"
#include "sensorData.h"
#ifndef DISABLE_WIFI
extern QueueHandle_t qJsonToNet;
#endif

// Пороговые константы взяты из старого скетча
static constexpr float V_CONNECTED_MIN = 5.0f;
static constexpr float I_CUTOFF = 0.5f;

void ChargerFSM::begin() {
    state_ = ChargeState::STANDBY;
    stateEntryMs_ = millis();
    DEBUG_INFO(F("ChargerFSM ready"));
}

void ChargerFSM::changeState(ChargeState newState) {
    state_ = newState;
    stateEntryMs_ = millis();
    Serial.printf("FSM → %s\n", getChargeStateName(state_));
}


const char *getChargeStateName(ChargeState state) {
    switch (state) {
        case ChargeState::STANDBY:
            return "STANDBY";
        case ChargeState::CHARGING:
            return "CHARGING";
        case ChargeState::PRECHARGE:
            return "PRECHARGE";
        case ChargeState::CC:
            return "CC";
        case ChargeState::CV:
            return "CV";
        case ChargeState::CHARGED:
            return "CHARGED";
        case ChargeState::ERROR:
            return "ERROR";
        case ChargeState::EMERGENCY_STOP:
            return "EMERGENCY_STOP";
        case ChargeState::BAD_CONNECTION:
            return "BAD_CONNECTION";
        case ChargeState::CHECK_CONNECTION:
            return "CHECK_CONNECTION";
        default:
            return "UNKNOWN";
    }
}

void ChargerFSM::updateTemperatureData(const SensorData &s) {
    if (!tempData_.isInitialized) {
        tempData_.ambient = s.tempAmbient;
        tempData_.dc = s.tempDC;
        tempData_.positive = s.tempPos;
        tempData_.negative = s.tempNeg;
        tempData_.isInitialized = true;
        return;
    }

    tempData_.ambient = s.tempAmbient;
    tempData_.dc = s.tempDC;
    tempData_.positive = s.tempPos;
    tempData_.negative = s.tempNeg;
}

bool ChargerFSM::hasTemperatureChanged() {
    if (!tempData_.isInitialized)
        return false;

    float deltaAmbient = abs(tempData_.ambient - tempData_.ambient);
    float deltaDC = abs(tempData_.dc - tempData_.dc);
    float deltaPositive = abs(tempData_.positive - tempData_.positive);
    float deltaNegative = abs(tempData_.negative - tempData_.negative);

    return deltaAmbient > TEMP_DELTA_THRESHOLD ||
           deltaDC > TEMP_DELTA_THRESHOLD ||
           deltaPositive > TEMP_DELTA_THRESHOLD ||
           deltaNegative > TEMP_DELTA_THRESHOLD;
}

bool ChargerFSM::checkTemperature() {
    return tempData_.ambient > TEMP_THRESHOLD ||
           tempData_.dc > TEMP_THRESHOLD ||
           tempData_.positive > TEMP_THRESHOLD ||
           tempData_.negative > TEMP_THRESHOLD;
}

bool ChargerFSM::checkConnectionQuality(const SensorData &s) {
    return s.voltage > BAD_CONNECTION_VOLTAGE;
}

bool ChargerFSM::checkCurrentFlow(const SensorData &s) {
    float currentThreshold =
        robotOn_ ? CURRENT_THRESHOLD_ROBOT_ON : CURRENT_THRESHOLD_ROBOT_OFF;
    return s.current < currentThreshold && s.current > I_CUTOFF;
}

void ChargerFSM::update(const SensorData &s) {
    // Обновление данных о температуре
    updateTemperatureData(s);

    // Проверка соединения
    bool nowConnected = (s.voltage > V_CONNECTED_MIN);
    if (nowConnected && !connected_) {
        connectionTimeMs_ = 0;
        connected_ = true;
    } else if (!nowConnected && connected_) {
        connected_ = false;
        changeState(ChargeState::STANDBY);
    }

    // Обновление времени
    uint32_t dt = millis() - stateEntryMs_;
    if (connected_)
        connectionTimeMs_ += dt;
    if (isCharging())
        chargingTimeMs_ += dt;
    stateEntryMs_ = millis();

    // Проверка аварийных ситуаций
    if (checkTemperature()) {
        emergencyStatus_ = EmergencyStatus::TEMPERATURE_PROBLEM;
        changeState(ChargeState::EMERGENCY_STOP);
        return;
    }

    if (chargingTimeMs_ > MAX_CHARGING_TIME_MS) {
        emergencyStatus_ = EmergencyStatus::LONG_CHARGING_TIME;
        changeState(ChargeState::EMERGENCY_STOP);
        return;
    }

    if (checkConnectionQuality(s)) {
        emergencyStatus_ = EmergencyStatus::CURRENT_FLOW_PROBLEM;
        changeState(ChargeState::BAD_CONNECTION);
        return;
    }

    // Основная логика состояний
    switch (state_) {
        case ChargeState::STANDBY:
            if (s.voltage > V_CONNECTED_MIN) {
                changeState(ChargeState::CHECK_CONNECTION);
            }
            break;

        case ChargeState::CHECK_CONNECTION:
            if (s.voltage > V_CONNECTED_MIN) {
                robotOn_ = s.current > CURRENT_THRESHOLD_ROBOT_ON;
                changeState(ChargeState::CHARGING);
            } else {
                changeState(ChargeState::STANDBY);
            }
            break;

        case ChargeState::CHARGING:
            if (checkCurrentFlow(s)) {
                changeState(ChargeState::CHARGED);
            }
            break;

        case ChargeState::CHARGED:
            if (!connected_) {
                changeState(ChargeState::STANDBY);
            }
            break;

        case ChargeState::EMERGENCY_STOP:
            if (!checkTemperature() && !checkConnectionQuality(s)) {
                emergencyStatus_ = EmergencyStatus::NONE;
                changeState(ChargeState::STANDBY);
            }
            break;

        case ChargeState::BAD_CONNECTION:
            if (s.voltage < V_CONNECTED_MIN) {
                changeState(ChargeState::STANDBY);
            }
            break;

        case ChargeState::ERROR:
            break;
    }

#ifndef DISABLE_WIFI
    JsonDocument doc;
    s.toJson(doc);
    doc["isConnected"] = nowConnected;
    doc["isCharging"] = isCharging();
    doc["chargingTime"] = chargingTimeMs_;
    doc["connectionTime"] = connectionTimeMs_;
    doc["currentMillis"] = 0;
    doc["chargingStatus"] = chargeStateToString(state_);
    doc["emergencyStatus"] = static_cast<int>(emergencyStatus_);
    doc["robotOn"] = robotOn_;
    doc["temperature"] = tempData_.ambient;
    xQueueSend(qJsonToNet, &doc, 0);
#endif
}
