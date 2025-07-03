#ifndef CHARGER_FSM_H
#define CHARGER_FSM_H

#include <Arduino.h>

#include "sensorReader.h"

enum class ChargeState : uint8_t {
    STANDBY,
    CHARGING,
    PRECHARGE,
    CC,
    CV,
    CHARGED,
    DISCONNECTED,
    ERROR,
    EMERGENCY_STOP,
    BAD_CONNECTION,
    CHECK_CONNECTION
};

enum class EmergencyStatus : uint8_t {
    NONE,
    TEMPERATURE_PROBLEM,
    CURRENT_FLOW_PROBLEM,
    LONG_CHARGING_TIME,
    LOW_CURRENT,
    HOT_FIRE
};

struct TemperatureData {
    float ambient;
    float dc;
    float positive;
    float negative;
    bool isInitialized;
};

class ChargerFSM {
public:
    void begin();
    void update(const SensorData &s);
    ChargeState state() const {
        return state_;
    }
    EmergencyStatus emergencyStatus() const {
        return emergencyStatus_;
    }

    bool isConnected() const {
        return connected_;
    }
    bool isCharging() const {
        return state_ == ChargeState::CC || state_ == ChargeState::CV;
    }
    uint32_t chargingTime() const {
        return chargingTimeMs_ / 1000;
    }
    uint32_t connectionTime() const {
        return connectionTimeMs_ / 1000;
    }
    bool isRobotOn() const {
        return robotOn_;
    }
    const TemperatureData &temperatureData() const {
        return tempData_;
    }

private:
    ChargeState state_ = ChargeState::STANDBY;
    EmergencyStatus emergencyStatus_ = EmergencyStatus::NONE;
    bool connected_ = false;
    bool robotOn_ = false;
    uint32_t stateEntryMs_ = 0;
    uint32_t chargingTimeMs_ = 0;
    uint32_t connectionTimeMs_ = 0;
    TemperatureData tempData_ = {0, 0, 0, 0, false};

    // Константы для контроля
    static constexpr float V_CONNECTED_MIN = 5.0f;
    static constexpr float I_CUTOFF = 0.5f;
    static constexpr float MAX_CHARGING_TIME_MS = 3600000;  // 1 час
    static constexpr float TEMP_THRESHOLD = 45.0f;
    static constexpr float TEMP_DELTA_THRESHOLD = 15.0f;
    static constexpr float BAD_CONNECTION_VOLTAGE = 40.0f;
    static constexpr float CURRENT_THRESHOLD_ROBOT_ON = 2.0f;
    static constexpr float CURRENT_THRESHOLD_ROBOT_OFF = 1.5f;
    static constexpr uint32_t CHECK_CONNECTION_DELAY_MS = 1000;

    void changeState(ChargeState newState);
    void manageRelays(const SensorData &s);
    bool checkTemperature();
    bool checkConnectionQuality(const SensorData &s);
    bool checkCurrentFlow(const SensorData &s);
    void updateTemperatureData(const SensorData &s);
    bool hasTemperatureChanged();
};
#endif
