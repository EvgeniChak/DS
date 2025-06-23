#include <ArduinoJson.h>
#include "BuildVersion.h"

String BuildVersion::m_sVersion = "";

BuildVersion::BuildVersion()
{
    m_sVersion.clear();
}

String BuildVersion::getRawJson()
{
    String newVersion(sVersion);
    newVersion.replace("\"", "\\\"");
    return newVersion;    
}

String BuildVersion::getVersion()
{
    //
    // No need to rebuild is created
    if (!m_sVersion.isEmpty())
        return m_sVersion;

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, sVersion);
    if (error) {
      Serial.printf("Error in Version json : %s\n", error.c_str());
      return "No Version";
    }

    //
    // Extract values from the JSON object
    int major = doc["major"];
    int minor = doc["minor"];
    int build = doc["build"];
    const char *creation = doc["creation"];

    //
    // Remember the version
    char sBuf[256];
    sprintf(sBuf, "%d.%d.%d - Created: %s", major, minor, build, creation );
    m_sVersion = String(sBuf);

    return m_sVersion;
}
