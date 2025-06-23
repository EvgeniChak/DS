#pragma once
#include <ArduinoJson.h>

/**
 * @brief Хранит все мгновенные показания датчиков.
 *
 * Поля совпадают по именам с теми, что раньше уходили в JSON.
 */
struct SensorData {
    float voltage = 0.0f;          ///< Напряжение батареи, В
    float voltageContacts = 0.0f;  ///< Напряжение контактов, В
    float voltageSocket = 0.0f;    ///< Напряжение контактов, В
    float current = 0.0f;          ///< Ток зарядки, А  (положительный — заряд)
    float capacity = 0.0f;         ///< Пока не рассчитывается → 0
    float tempAmbient = 0.0f;      ///< Температура внутри корпуса, °C
    float tempDC = 0.0f;           ///< Температура DC‑разъёма, °C
    float tempNeg = 0.0f;          ///< Температура «‑» клеммы, °C
    float tempPos = 0.0f;          ///< Температура «+» клеммы, °C
    unsigned long millisTs = 0;    ///< Время сбора данных, мс

    /**
     * @brief Записывает все поля в JSON строго в старой схеме.
     * @param[out] doc Доступный JsonDocument не меньше 256 байт.
     */
    void toJson(JsonDocument& doc) const {
        doc["voltage"] = voltage;
        doc["current"] = current;
        doc["capacity"] = capacity;

        JsonArray temps = doc["Temperatures"].to<JsonArray>();
        temps.add(tempAmbient);
        temps.add(tempDC);
        temps.add(tempNeg);
        temps.add(tempPos);
    }
};
