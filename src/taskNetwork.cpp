#ifndef DISABLE_WIFI
#include <ArduinoJson.h>
#include <WiFi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include "ledController.h"
#include "mqtt_utils.h"
#include "state_machine.h"
#include "wifi_utils.h"

extern QueueHandle_t qJsonToNet;  // объявлен в main.cpp
extern MQTTClient client;         // из mqtt_utils
static LedController gNetLED;

/* ---------- задача сети ---------- */
void taskNetwork(void* /*pv*/) {
    gNetLED.begin();
    StaticJsonDocument<256> doc;

    for (;;) {
        runStateMachine();  // Wi‑Fi, портал, MQTT keep‑alive

        if (WiFi.status() != WL_CONNECTED || !client.connected())
            gNetLED.netError();
        else
            gNetLED.netOk();

        if (xQueueReceive(qJsonToNet, &doc, 0) == pdPASS) {
            String payload;
            serializeJson(doc, payload);
            client.publish("/docking_station/to_server/status", payload);
        }

        gNetLED.loop();
        vTaskDelay(pdMS_TO_TICKS(200));  // 5 Гц
    }
}
#endif
