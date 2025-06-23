#include "OTA.h"
#include "ServerComms.h"
#include "Update.h"

// static String URL = "https://t1staging.xtendrobotics.com/api/storage/download/xtend-arduino%2F";
static String URL = "https://t1staging.xtendrobotics.com/api/storage/download/";

OTA::OTA(String sFile)
{
    m_sFile = sFile;
}

bool OTA::Start()
{
    //
    // Download the OTA file
    if (!DoDownload()) {
        Serial.println("OTA: Download failed...");
        return false;
    }

    //
    // Update the firmware
    if (!DoUpdate()) {
        Serial.println("OTA: Update failed...");
        return false;
    }

    //
    // Restarting the ESP
    Serial.println("Reset in 4 seconds....");
    delay(4000);
    ESP.restart(); // Restart ESP32 to apply the update
    
    return true;
}

bool OTA::DoDownload()
{
    //
    // Get the token
    ServerComms *pServer = ServerComms::getInstance();
    String sToken = pServer->GetToken();

    HTTPClient http;

    //
    // Send the GET to the server
    String sURL = URL + m_sFile;
    Serial.printf("File Download Path: %s\n", sURL.c_str());
    http.begin(sURL);
    http.addHeader("Authorization", sToken);
    int httpCode = http.GET();

    //
    // Process any redirection
    while (httpCode == HTTP_CODE_FOUND) {
        String location = http.getLocation();
        Serial.printf("REDIRECTED URL: %s\n", location.c_str());
        http.end();
        http.begin(location);
        httpCode = http.GET();
    }

    if (httpCode != HTTP_CODE_OK) { 
        Serial.printf("Accessing Server failed, Error: %d\n", httpCode);
        return false;
    }

    //
    // Create the download file
    String sFile = "/firmware.bin";
    File file = SPIFFS.open(sFile, FILE_WRITE);
    if (!file) {
      Serial.printf("Failed to open output file %s for writing", sFile);
      return false;
    }

    //
    // Start downloading..
    WiFiClient* stream = http.getStreamPtr();
    uint8_t buffer[1 * 1024];
    int totalBytes = 0;

    while (http.connected() || stream->available()) {
      int bytesRead = stream->readBytes(buffer, sizeof(buffer));
      if (bytesRead > 0) {
        file.write(buffer, bytesRead);
        totalBytes += bytesRead;
        Serial.printf("Downloaded %d bytes\n", totalBytes);
      }
    }

    //
    // Close up
    file.close();
    Serial.printf("Download complete! Total size: %d bytes\n", totalBytes);
    http.end();

    return true;
}

bool OTA::DoUpdate()
{
    //
    // Open the firmware file that was downloaded in SPIFFS for reading
    File file = SPIFFS.open("/firmware.bin", FILE_READ);
    if (!file) {
        Serial.println("Failed to open downloaded file for reading");
        return false;
    }   

    //
    // Begin the update
    size_t fileSize = file.size(); // Get the file size
    Serial.printf("Starting update, %d\n", fileSize);
    if (!Update.begin(fileSize, U_FLASH)) {
        Serial.println("Failed to do the update.. ");
        return false;
    }

    Update.writeStream(file);

    //
    // Complete the OTA update process
    if (Update.end()) {
        Serial.println("Successful OTA update!! ");
    }
    else {
        Serial.println("Error Occurred:" + String(Update.getError()));
        return false;
    }

    file.close(); // Close the file
    return true;
}