#pragma once
#include <FastLED.h>

#include "chargerFSM.h"  // для ChargeState
#include "config.h"

class LedController {
public:
    void begin(uint8_t brightness = 64);

    /* --- режимы статуса зарядки --- */
    void update(ChargeState st);  // ← вызывается из main.cpp

    /* --- сетевой статус --- */
    void netError();  // мигаем красным
    void netOk();     // постоянный зелёный

    /* --- внутреннее обслуживание мигания --- */
    void loop();  // вызывать в цикле задачи LED
    inline void updateBlink() {
        loop();
    }  // алиас, если где‑то использовался

private:
    /* helpers */
    void setAll(CRGB c);
    void toggleBlink(uint16_t periodMs);
    void disableBlink();

    /* state */
    // static constexpr uint8_t LED_COUNT = 1;
    CRGB leds_[LED_COUNT];
    uint32_t blinkPeriodMs_ = 0;
    uint32_t lastToggleMs_ = 0;
    bool ledOn_ = true;
};
