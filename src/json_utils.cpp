#include <FS.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

#include "json_utils.h"
#include "mqtt_utils.h"
#include "certificate.h"
#include "ServerComms.h"
#include "wifi_utils.h"

bool loadMQTTConfig(const char* path) {
    if (!SPIFFS.exists(path)) {
        Serial.println("TLS configuration file does not exist");
        return false;
    }

    File file = SPIFFS.open(path, "r");
    if (!file) {
        Serial.println("Failed to open file for reading");
        return false;
    }

    size_t size = file.size();
    std::unique_ptr<char[]> buf(new char[size]);
    file.readBytes(buf.get(), size);
    file.close();

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, buf.get());
    if (error) {
        Serial.println("Failed to parse JSON: ");
        Serial.println(error.c_str());
        return false;
    }
    dsId = doc["id"].as<String>();
    String privateKey = doc["privateKey"].as<String>();
    String certificate = doc["certificate"].as<String>();

    tlsClient.setCACert(AmazonRootCA);
    tlsClient.setPrivateKey(privateKey.c_str());
    tlsClient.setCertificate(certificate.c_str());

    String Token = doc["token"].as<String>();
    ServerComms * Server = ServerComms::getInstance();
    Server->SetToken(Token);

    Serial.print("TLS Config loaded from file for deviceID: ");
    Serial.println(dsId);
    return true;
}

void saveMQTTConfig(const String& jsonString, const char* path) {
    File file = SPIFFS.open(path, "w");
    if (!file) {
        Serial.println("Failed to open file for writing");
        return;
    }

    file.print(jsonString);
    file.close();
}

bool loadWiFiCredentials() {
    if (!SPIFFS.exists("/WiFi.JSON")) {
        return false;
    }

    File file = SPIFFS.open("/WiFi.JSON", "r");
    if (!file) {
        Serial.println("Failed to open WiFi credentials file");
        return false;
    }

    size_t size = file.size();
    std::unique_ptr<char[]> buf(new char[size]);
    file.readBytes(buf.get(), size);
    file.close();

    // DynamicJsonDocument doc(1024);
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, buf.get());
    if (error) {
        Serial.println("Failed to parse WiFi JSON: ");
        Serial.println(error.c_str());
        return false;
    }

    ssid = doc["ssid"].as<String>();
    pass = doc["password"].as<String>();
    acti = doc["activationCode"].as<String>();

    return true;
}

bool saveWiFiCredentials(const char* ssid, const char* password, const char* acticode) {
    // Create a JSON document to store the credentials
    // DynamicJsonDocument doc(1024);
    JsonDocument doc;

    // Set the JSON document with the provided ssid and password
    doc["ssid"] = ssid;
    doc["password"] = password;
    doc["activationCode"] = acticode;

    // Open the WiFi.JSON file for writing
    File file = SPIFFS.open("/WiFi.JSON", "w");
    if (!file) {
        Serial.println("Failed to open WiFi credentials file for writing");
        return false;
    }

    // Serialize the JSON document to the file
    if (serializeJson(doc, file) == 0) {
        Serial.println("Failed to write WiFi credentials to file");
        file.close();
        return false;
    }

    // Close the file
    file.close();
    Serial.println("WiFi credentials saved successfully");
    return true;
}
