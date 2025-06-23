#ifndef JSON_UTILS_H
#define JSON_UTILS_H

#include <ArduinoJson.h>

bool loadMQTTConfig(const char* path);
void saveMQTTConfig(const String& jsonString, const char* path);
bool loadWiFiCredentials();
bool saveWiFiCredentials(const char* ssid, const char* password, const char* acticode);


#endif // JSON_UTILS_H
