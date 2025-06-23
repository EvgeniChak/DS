#ifndef SENSOR_READER_H
#define SENSOR_READER_H

#include <Arduino.h>
#include "sensorData.h"
#include <array>


class SensorReader {
public:
    void begin();
    void update();
    const SensorData &get() const {
        return data_;
    }

private:
    SensorData data_;
    float readVoltage(uint8_t pin);
    float readCurrent(uint8_t pin);
    float readNTC(uint8_t pin);
};
#endif
