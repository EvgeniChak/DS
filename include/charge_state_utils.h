#ifndef CHARGE_STATE_UTILS_H
#define CHARGE_STATE_UTILS_H

#include "ChargerFSM.h"

/** Преобразует ChargeState в строку для MQTT/WS. */
static inline const char* chargeStateToString(ChargeState s) {
    switch (s) {
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
        case ChargeState::DISCONNECTED:
            return "DISCONNECTED";
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

#endif  // CHARGE_STATE_UTILS_H
