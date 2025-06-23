#include "state_machine.h"

#include <Arduino.h>
#include <ArduinoJson.h>

#include "ServerComms.h"
#include "WiFi.h"
#include "json_utils.h"
#include "mqtt_utils.h"
#include "wifi_utils.h"

#define MINUTES(m)  (m * 60 * 1000)

void runStateMachine() {
    ServerComms *server = ServerComms::getInstance();
    State ConnType = WEBSOCKET_CONNECT;
    switch (currentState) {
        case INIT:
            Serial.println("State: INIT");
            currentState = WIFI_CONNECT; 
            // TODO: wait for long button press during boot to jump to WIFI_MANAGER
            break;

        case WIFI_CONNECT:
            Serial.println("State: WIFI_CONNECT");
            if (connectWiFi()) {
                currentState = ACTIVATION;
                Serial.println("State: WIFI_CONNECT ok, go to state ACTIVATION");
            } else {
                currentState = WIFI_MANAGER; // If WiFi connection fails, go to WiFi manager
                Serial.println("State: WIFI_CONNECT wifi connection fail, go to state WIFI_MANAGER");
            }
            break;

        case ACTIVATION:
            Serial.println("State: ACTIVATION");
            // Attempt to load the MQTT configuration
            if (loadMQTTConfig("/MQTT.JSON")) {
                Serial.println("State: ACTIVATION, OK. TLS certificates loaded from file.");
                currentState = ConnType;
            } else {
                Serial.println("State: ACTIVATION, Failed to load MQTT/websocket configuration, attempting to obtain new certificates.");
                // Attempt to obtain new certificates using the activation code
                if (postActivationCode()) {
                    Serial.println("State: ACTIVATION, Success, obtained MQTT/websocket configuration through activation.");
                    currentState = ConnType;
                } else {
                    Serial.println("State: ACTIVATION, Failed to obtain MQTT/websocket configuration through activation.");
                    currentState = ConnType;
                }
            }
            break;
            
        case WEBSOCKET_CONNECT:
            Serial.println("State: WEBSOCKET_CONNECT");

            if (server->Connect()) {
                currentState = MQTT_PUBLISH;
                Serial.println("State: WEBSOCKET_CONNECT, ok. Go to state MQTT_PUBLISH");
            } else {
                currentState = WIFI_MANAGER; // Go to WiFi manager if connection fails
                Serial.println("State: WEBSOCKET_CONNECT, fail. Go to state WIFI_MANAGER");
            }
            break;
        


        case MQTT_CONNECT:
            Serial.println("State: MQTT_CONNECT");
            if (connectToMQTT()) {
                currentState = MQTT_PUBLISH;
                Serial.println("State: MQTT_CONNECT, ok. Go to state MQTT_PUBLISH");
            } else {
                currentState = WIFI_MANAGER; // Go to WiFi manager if MQTT connection fails
                Serial.println("State: MQTT_CONNECT, fail. Go to state WIFI_MANAGER");
            }
            break;

        case MQTT_PUBLISH:
            // Serial.println("State: MQTT_PUBLISH");
            // This state is handled in the main loop
            break;

        case WIFI_MANAGER:
            Serial.println("State: WIFI_MANAGER");
            setupWiFiPortal();
            dnsServer.processNextRequest(); 
            
            // check WiFi Scan Async procgress
            int16_t WiFiScanStatus = WiFi.scanComplete();
            Serial.print("WIfi Network Scan status: ");
            while (WiFiScanStatus < 0)
            {
                Serial.printf("%d, ", WiFiScanStatus);
                WiFiScanStatus = WiFi.scanComplete();
                delay(1000); //chill, otherwise async breaks
            }

            if (WiFiScanStatus >= 0) {  // Found Zero or more Wireless Networks
                processScannedNetworks(WiFiScanStatus);
            } 
 
            //
            // timeot to run portal if SSID exists
            unsigned long timeout = millis();

            while (currentState == WIFI_MANAGER) { // the only way out is a reset

                dnsServer.processNextRequest(); 

                //
                // If we have an SSID then stay here for 5 miutes before trying again to reconnect to the current SSID
                if (bHasSSId) {
                    if (millis() - timeout  > MINUTES(5) || bHasNewSSID) {
                        Serial.println("SSID received - going back to WIFI_CONNECT");
                        webServer.end();
                        currentState = WIFI_CONNECT;
                    }      
                }

                delay(10);
            }

            break;
    }
}
