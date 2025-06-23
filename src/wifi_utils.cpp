#include "wifi_utils.h"

#include <DNSServer.h>
#include <ESPAsyncWebServer.h>

#include "WiFi.h"
#include "json_utils.h"

String ssid;
String pass;
String acti;
bool bHasSSId = false;
bool bHasNewSSID = false;

// Portal Constants
const char *apSSID = "XTENDCHARGER";
const int apChannel = random(1, 12);
const byte DNS_PORT = 53;

AsyncWebServer webServer(80);
DNSServer dnsServer;
String networksJsonString = "";

const char *html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Xtend WiFi Charger Configuration</title>
  <script>
    document.addEventListener('DOMContentLoaded', async function() {
      await scanNetworks();
    });

    async function scanNetworks() {
      const response = await fetch('/scan');
      const networks = await response.json();
      
      let networksHtml = `
        <form id="wifiForm">
          <table border="1" cellpadding="10" cellspacing="0">
            <thead>
              <tr>
                <th>Select</th>
                <th>SSID</th>
                <th>RSSI (dBm)</th>
                <th>Signal Strength (%)</th>
                <th>Encryption</th>
              </tr>
            </thead>
            <tbody>
      `;

      networks.networks.forEach((network, index) => {
        networksHtml += `
          <tr>
            <td><input type="radio" id="network${index}" name="ssid" value="${network.ssid}"></td>
            <td><label for="network${index}">${network.ssid}</label></td>
            <td>${network.rssi}</td>
            <td>${network.signal}</td>
            <td>${network.encryption}</td>
          </tr>
        `;
      });

      networksHtml += `
            </tbody>
          </table><br>
          <label for="password"><b>WiFi password:</b></label>
          <input type="text" id="password" name="password" required><br><br>
          <label for="acti"><b>Device activation code:</b></label>
          <input type="text" id="acti" name="acti" required><br><br>
          <button type="button" onclick="submitConfig()">Save Configuration</button>
        </form>
      `;

      document.getElementById('networks').innerHTML = networksHtml;
    }

    async function submitConfig() {
      const form = document.getElementById('wifiForm');
      const formData = new FormData(form);
      const ssid = formData.get('ssid');
      const password = formData.get('password');
      const acti = formData.get('acti');

      if (!ssid) {
        alert('Please select a Wifi network');
        return;
      }
      if (!password) {
        alert('Please fill in Wifi password');
        return;
      }
      if (!acti || !/^\d{8}$/.test(acti)) {
        alert('Please fill in a valid 8-digit device activation code');
        return;
      }

      const response = await fetch('/config', {
        method: 'POST',
        headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
        body: `ssid=${encodeURIComponent(ssid)}&password=${encodeURIComponent(password)}&acti=${encodeURIComponent(acti)}`
      });

      if (response.ok) {
        alert(await response.text());
        window.location.reload();
      } else {
        alert('Failed to save configuration');
      }
    }
  </script>
</head>
<body>
  <h1>Xtend WiFi Charger Configuration</h1>
  <div id="networks"></div>
</body>
</html>
)rawliteral";

void processScannedNetworks(uint16_t networksFound) {
    JsonDocument jsonPortalDoc;
    JsonArray networks = jsonPortalDoc.createNestedArray("networks");
    // JsonArray networks = jsonPortalDoc["networks"];

    if (networksFound > 0) {
        Serial.printf("\nWiFI scan: %d networks found\n", networksFound);

        for (int i = 0; i < networksFound; ++i) {
            JsonObject network = networks.createNestedObject();
            network["ssid"] = WiFi.SSID(i);
            network["rssi"] = WiFi.RSSI(i);
            network["signal"] =
                map(WiFi.RSSI(i), -100, -50, 0,
                    100);  // Convert RSSI to signal strength percentage

            switch (WiFi.encryptionType(i)) {
                case WIFI_AUTH_OPEN:
                    network["encryption"] = "open";
                    break;
                case WIFI_AUTH_WEP:
                    network["encryption"] = "WEP";
                    break;
                case WIFI_AUTH_WPA_PSK:
                    network["encryption"] = "WPA";
                    break;
                case WIFI_AUTH_WPA2_PSK:
                    network["encryption"] = "WPA2";
                    break;
                case WIFI_AUTH_WPA_WPA2_PSK:
                    network["encryption"] = "WPA+WPA2";
                    break;
                case WIFI_AUTH_WPA2_ENTERPRISE:
                    network["encryption"] = "WPA2-EAP";
                    break;
                case WIFI_AUTH_WPA3_PSK:
                    network["encryption"] = "WPA3";
                    break;
                case WIFI_AUTH_WPA2_WPA3_PSK:
                    network["encryption"] = "WPA2+WPA3";
                    break;
                case WIFI_AUTH_WAPI_PSK:
                    network["encryption"] = "WAPI";
                    break;
                default:
                    network["encryption"] = "unknown";

                    //        networks.add(network);
            }
        }

        serializeJson(jsonPortalDoc, networksJsonString);
        Serial.print("processScannedNetworks:");
        Serial.println(networksJsonString);

        // Delete the scan result to free memory
        WiFi.scanDelete();
    }
}

bool connectWiFi() {
    // Load WiFi credentials from JSON or go to generate new ones
    if (loadWiFiCredentials()) {
        bHasSSId = true;
        WiFi.mode(WIFI_STA);
        Serial.print("Connecting to WiFi SSID: ");
        Serial.println(ssid.c_str());
        WiFi.begin(ssid.c_str(), pass.c_str());

        unsigned long startAttemptTime = millis();
        Serial.print("Wifi Connection Status: ");
        while (WiFi.status() != WL_CONNECTED &&
               millis() - startAttemptTime < 45000) {
            delay(500);
            Serial.printf("%d, ", WiFi.status());

            //
            // if connection fails, then retry connecting
            if (WiFi.status() == WL_CONNECT_FAILED)
                WiFi.begin(ssid.c_str(), pass.c_str());
        }
    } else {
        Serial.println("Cant load wifi credentials from file");
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnected to WiFi");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        return true;
    } else {
        Serial.println("\nFailed to connect to WiFi");
        return false;
    }
}

void setupWiFiPortal() {
    bHasSSId = false;
    Serial.println("Setting up WiFi Manager Portal...");

    // Start Access Point
    WiFi.softAP(apSSID, nullptr, apChannel);
    Serial.print("Access Point started with SSID: ");
    Serial.println(apSSID);

    // Start DNS server
    dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

    // initialize wifi network list generation in async mode
    WiFi.disconnect();
    WiFi.scanNetworks(true, false);

    // Set up web webServer routes
    webServer.on("/", HTTP_GET, portalHandleRoot);
    webServer.on("/scan", HTTP_GET, portalHandleScan);
    webServer.on("/config", HTTP_POST, portalHandleConfig);
    webServer.onNotFound(
        portalHandleRoot);  // Redirect all other requests to the captive portal
    webServer.begin();

    Serial.println("HTTP Server started");
}

void portalHandleRoot(AsyncWebServerRequest *request) {
    request->send(200, "text/html", html);
}

void portalHandleScan(AsyncWebServerRequest *request) {
    request->send(200, "application/json", networksJsonString);
}

void portalHandleConfig(AsyncWebServerRequest *request) {
    if (request->hasParam("ssid", true) &&
        request->hasParam("password", true) &&
        request->hasParam("acti", true)) {
        const char *ssid = request->getParam("ssid", true)->value().c_str();
        const char *password =
            request->getParam("password", true)->value().c_str();
        const char *activation =
            request->getParam("acti", true)->value().c_str();
        if (saveWiFiCredentials(ssid, password, activation)) {
            request->send(200, "text/html",
                          "Configuration saved. Rebooting...");
            Serial.println("Webserver received: ");
            Serial.println(ssid);
            Serial.println(password);
            Serial.println(activation);

            delay(2000);
            bHasNewSSID = true;
            bHasSSId = true;
            // ESP.restart();
        } else {
            request->send(400, "text/plain", "Missing parameters");
        }
    }
}