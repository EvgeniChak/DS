#include <Arduino.h>
#include <ArduinoJson.h>
#include <esp_bt.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include "charge_state_utils.h"
#include "chargerFSM.h"
#include "config.h"
#include "ledController.h"
#include "logger.h"
#include "sensorReader.h"

#ifndef DISABLE_WIFI
#include "ServerComms.h"
#include "state_machine.h"
#endif

extern void taskNetwork(void *);
void disableBluetooth();

#ifndef DISABLE_WIFI
State currentState = INIT;
#endif
SensorReader gSensors;
ChargerFSM gFSM;
LedController gLED;
LedController gNetLED;

#ifndef DISABLE_WIFI
extern QueueHandle_t qJsonToNet;  // переобъявим здесь
QueueHandle_t qJsonToNet = nullptr;
#endif

/* ---------- FreeRTOS tasks ---------- */
void taskSensor(void *) {
    gSensors.begin();
    for (;;) {
        gSensors.update(gFSM.state());
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void taskFSM(void *) {
    gFSM.begin();
    for (;;) {
        gFSM.update(gSensors.getData());
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void taskLED(void *) {
    gLED.begin();
    for (;;) {
        gLED.loop();
        gLED.update(gFSM.state());
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void setup() {
    DEBUG_INFO(F("Starting setup..."));
    initLogger();
    DEBUG_INFO(F("Logger initialized"));

    randomSeed(micros());
    DEBUG_INFO(F("Random seed initialized"));

    DEBUG_INFO(F("Configuring pins..."));
    pinMode(PIN_RELAY1, OUTPUT);
    pinMode(PIN_RELAY2, OUTPUT);
    // pinMode(PIN_LED_STRIP, OUTPUT);
    pinMode(PIN_VDIV1, INPUT);
    pinMode(PIN_VDIV2, INPUT);
    pinMode(PIN_CURRENT, INPUT);
    pinMode(PIN_TEMP_AMBIENT, INPUT);
    // pinMode(PIN_TEMP_DC, INPUT);
    pinMode(PIN_TEMP_NEG, INPUT);
    pinMode(PIN_TEMP_POS, INPUT);
    DEBUG_INFO(F("Pins configured"));
    analogReadResolution(12);
    digitalWrite(PIN_RELAY1, LOW);
    digitalWrite(PIN_RELAY2, LOW);
    DEBUG_INFO(F("Relays set to LOW"));

#ifndef DISABLE_WIFI
    DEBUG_INFO(F("Creating queue..."));
    qJsonToNet = xQueueCreate(4, sizeof(StaticJsonDocument<256>));
    if (qJsonToNet == nullptr) {
        DEBUG_ERROR(F("Failed to create queue!"));
    } else {
        DEBUG_INFO(F("Queue created successfully"));
    }

    DEBUG_INFO(F("Creating network task..."));
    xTaskCreatePinnedToCore(taskNetwork, "Network", 8192, nullptr, 2, nullptr,
                            0);
#endif

    DEBUG_INFO(F("Creating tasks..."));
    xTaskCreatePinnedToCore(taskSensor, "Sensor", 4096, nullptr, 2, nullptr, 1);
    xTaskCreatePinnedToCore(taskFSM, "FSM", 4096, nullptr, 3, nullptr, 1);
    xTaskCreatePinnedToCore(taskLED, "LED", 4096, nullptr, 1, nullptr, 0);
    DEBUG_INFO(F("Tasks created"));

    DEBUG_INFO(F("Setup completed"));
}

void loop() {
#ifndef DISABLE_WIFI
    ServerComms::getInstance()->Poll();

    /* Если Wi‑Fi упал — перезапускаем стейт‑машину */
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Lost Wi‑Fi, reconnecting…");
        currentState = WIFI_CONNECT;
        while (currentState != MQTT_PUBLISH)
            runStateMachine();
    }

    delay(20);
#endif
}
