#ifndef MQTT_UTILS_H
#define MQTT_UTILS_H

#include <MQTTClient.h>
#include <WiFiClientSecure.h>

extern MQTTClient client;
extern WiFiClientSecure tlsClient;
extern String dsId; //device ID

bool postActivationCode(); 
bool connectToMQTT();
void messageHandler(String &topic, String &payload);

#endif // MQTT_UTILS_H
