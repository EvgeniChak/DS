#include "ledController.h"

#include "config.h"  // PIN_LED_STRIP

void LedController::begin(uint8_t brightness) {
    FastLED.addLeds<WS2812, PIN_LED_STRIP, RGB>(leds_, LED_COUNT);
    FastLED.setBrightness(brightness);
    setAll(CRGB::Black);
    FastLED.show();
}

/* ---------- RUNTIME ---------- */
void LedController::setAll(CRGB c) {
    for (auto &p : leds_)
        p = c;
}

void LedController::update(ChargeState st) {
    switch (st) {
        case ChargeState::DISCONNECTED:
        case ChargeState::STANDBY:
            setAll(CRGB::Yellow);
            break;
        case ChargeState::CHECK_CONNECTION:
        case ChargeState::CHARGING:
            setAll(CRGB::Blue);
            break;
        case ChargeState::CHARGED:
            setAll(CRGB::Green);
            break;
        case ChargeState::ERROR:
            setAll(CRGB::Red);
            break;
        default:
            setAll(CRGB::Black);
            break;
    }
    FastLED.show();
}

/* ---------- blinking helpers ---------- */
void LedController::toggleBlink(uint16_t periodMs) {
    blinkPeriodMs_ = periodMs;
    lastToggleMs_ = millis();
    ledOn_ = true;
}
void LedController::disableBlink() {
    blinkPeriodMs_ = 0;
    ledOn_ = true;
}
void LedController::loop() {
    if (!blinkPeriodMs_)
        return;
    uint32_t now = millis();
    if (now - lastToggleMs_ >= blinkPeriodMs_) {
        lastToggleMs_ = now;
        ledOn_ = !ledOn_;
        if (ledOn_)
            FastLED.show();
        else {
            setAll(CRGB::Black);
            FastLED.show();
        }
    }
}

/* ---------- network wrappers ---------- */
void LedController::netError() {
    setAll(CRGB::Red);
    toggleBlink(300);
}
void LedController::netOk() {
    setAll(CRGB::Green);
    disableBlink();
}
