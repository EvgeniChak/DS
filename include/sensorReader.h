#ifndef SENSOR_READER_H
#define SENSOR_READER_H

#include <Arduino.h>

#include <array>

#include "sensorData.h"
#include "ChargerFSM.h"

 enum class ChargeState : uint8_t;  // Forward declaration

class SensorReader {
public:
    void begin();
    void update(ChargeState currentState);
    const SensorData &getData() {
        return data_;
    }

private:
    SensorData data_;
    float readVoltage(uint8_t pin);
    float readCurrent(uint8_t pin);
    float readNTC(uint8_t pin);
};
#endif
