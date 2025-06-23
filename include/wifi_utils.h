#ifndef WIFI_UTILS_H
#define WIFI_UTILS_H

#include <ESPAsyncWebServer.h>
#include <DNSServer.h>

extern DNSServer dnsServer;
extern String ssid;
extern String pass;
extern String acti;
extern bool bHasSSId;
extern bool bHasNewSSID;
extern AsyncWebServer webServer;

bool connectWiFi();
void setupWiFiPortal();
void processScannedNetworks(uint16_t networksFound);
void portalHandleRoot(AsyncWebServerRequest *request);
void portalHandleScan(AsyncWebServerRequest *request);
void portalHandleConfig(AsyncWebServerRequest *request);

#endif // WIFI_UTILS_H
