#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <MQTTClient.h> // lwmqtt - https://github.com/256dpi/arduino-mqtt
#include "mqtt_utils.h"
#include "certificate.h"
#include "json_utils.h"
#include "wifi_utils.h"
#include "ServerComms.h"



static const char* mqttServer = "a1n7w8x8peyqkq-ats.iot.eu-central-1.amazonaws.com"; // MQTT addrr
static const char* endpointBase = "https://t1staging.xtendrobotics.com/api/power/station/activate/"; // HTTPS activation endpoint addrr

WiFiClientSecure tlsClient;
MQTTClient client = MQTTClient(256); // global
String dsId; // global - device ID
 

// Function to handle activation by sending the activation code to the server
bool postActivationCode() {
    HTTPClient https;
    if (loadWiFiCredentials()) {
      String url = String(endpointBase) + acti;

      Serial.println("Query for TLS certificates");
      https.begin(url);
      int httpResponseCode = https.POST("");

      if (httpResponseCode == 200) {
          String response = https.getString();
          Serial.println("Received Response: " + response);

          // Parse JSON response
          JsonDocument doc;  // Increase the size if necessary
          deserializeJson(doc, response);

          bool isSuccess = doc["isSuccess"].as<bool>();
          String errorMessage = doc["error"].as<String>();

          if (isSuccess && errorMessage == "null") {
              dsId = doc["data"]["id"].as<String>();
              String privateKey = doc["data"]["privateKey"].as<String>();
              String certificate = doc["data"]["certificate"].as<String>();
              String token = doc["data"]["token"].as<String>();

              // Save configuration to SPIFFS
              String jsonString;
              serializeJson(doc["data"], jsonString);
              saveMQTTConfig(jsonString, "/MQTT.JSON");

              // Load keys and certificate into WiFiClientSecure
              tlsClient.setCACert(AmazonRootCA);
              tlsClient.setPrivateKey(privateKey.c_str());
              tlsClient.setCertificate(certificate.c_str());

              ServerComms * Server = ServerComms::getInstance();
              Server->SetToken(token);

              Serial.println("Private Key and Certificate saved to file");
              //Serial.println(jsonString);

              https.end();
              return true;
          } else {
              Serial.println("Error in response: " + errorMessage);
          }
      } else {
          Serial.println("HTTP POST failed with code: " + String(httpResponseCode));
      }
      https.end();
    } else {
      Serial.println("Cant load activation code from file");
    }

    return false;
}

bool connectToMQTT() 
{
    client.begin(mqttServer, 8883, tlsClient);  // dont put the port in a variable, it breaks everything!
    client.onMessage(messageHandler);

    Serial.print("Connecting to MQTT with client ID: ");
    Serial.println(dsId);

    String keepaliveTopic = "/docking_station/to_server/alive/" + dsId;
    // Set the Will message to "FALSE"- keepalive mechanism
    client.setWill(keepaliveTopic.c_str(), "false", 1, true);

    while (!client.connect(dsId.c_str())) {
        Serial.print('.');
        delay(100);
    }

    // Publish a retained message with "TRUE" - keepalive mechanism
    client.publish(keepaliveTopic.c_str(), "true", true, 1);

    if (client.connected()) {
        Serial.println("Connected to MQTT");
        return true;
    } else {
        Serial.print("MQTT Failed to connect, state: ");
        Serial.println(client.lastError());
        return false;
    }
}

void messageHandler(String &topic, String &payload) {
    Serial.println("incoming: " + topic + " - " + payload);
}
