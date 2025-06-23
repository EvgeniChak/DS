#include "ServerComms.h"
#include "certificate.h"
#include <ArduinoJson.h>
#include "BuildVersion.h"
#include "OTA.h"

static String Server = "t1staging.xtendrobotics.com";
// static String Server  = "192.168.1.117";

ServerComms * ServerComms::getInstance()
{
    static ServerComms * m_Instance = NULL;

    if (m_Instance != NULL)
        return m_Instance;

    m_Instance = new ServerComms;
    m_Instance->m_bConnected = false;
    m_Instance->m_bVersionSent = false;
    return m_Instance;
}


void ServerComms::SetToken(String Token)
{
    m_Token = Token;
}

void ServerComms::Poll()
{
    m_WSC.loop();
}

bool ServerComms::IsConnected()
{
    return m_bConnected;
}

void ServerComms::SendVersion()
{
    if (m_bVersionSent)
        return;
    
    BuildVersion vsn;
    if (sendEvent("version", vsn.getRawJson()))
        m_bVersionSent = true;
}

bool ServerComms::ProcessEvent(char *sEvent)
{
    Serial.printf("[WSc] ProcessEvent: %s\n", sEvent);

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, sEvent);
    if (error) {
      Serial.printf("Error parsing event to Json json : %s, %s\n", sEvent, error.c_str());
      return "No Version";
    }

    Serial.printf("Command: %s, payload: %s\n", (const char *)doc[0], (const char *)doc[1]);
    String sCommand = String((const char *)doc[0]);
    
    //
    // check for OTA Command 
    if (sCommand == "downloadFile") {
        OTA ota((const char *)doc[1]);
        ota.Start();    
    }

    return true;
}

void ServerComms::webSocketEvent(socketIOmessageType_t type, uint8_t * payload, size_t length) 
{
    switch(type) {
        case sIOtype_DISCONNECT:
            m_bConnected = false;
            Serial.printf("WebSocket Disconnected!\n");
            break;

        case sIOtype_CONNECT:
            m_bConnected = true;
            Serial.printf("WebSocket Connected to url: %s\n", payload);
            sendEvent("alive", "true");  // Let the server know we are alive.
            SendVersion();
            break;

        case sIOtype_EVENT:
            if (strcmp((char*)payload, "3") == 0) {
                Serial.println("[WSc] Received Socket.IO pong, sending ack");
				m_WSC.sendEVENT("3");
            } else if (strncmp((char*)payload, "42", 2) == 0) {
                Serial.printf("[WSc] Received Socket.IO event: %s\n", (char *)payload);
            }

            //
            // Process all other events coming in as ["Event", "payload"]
            else 
                ProcessEvent((char *)payload);
            break;
    }
}

bool ServerComms::Connect()
{
    //
    // Set the authorization headers
    Serial.printf("URL: %s\nToken: %s\n", Server.c_str(), m_Token.c_str());

    char Buf[2048];
    sprintf(Buf, "authorization: %s", m_Token.c_str());
    m_WSC.setExtraHeaders(Buf);

    //
    // Connect
//    m_WSC.setReconnectInterval(5000);
    m_WSC.beginSSL(Server.c_str(), 443, "/socket.io/?EIO=3","arduino", 10000, 20000);
//    m_WSC.begin(Server.c_str(), 8080, "/socket.io/?EIO=3","arduino", 10000, 20000);
    m_WSC.onEvent([this](socketIOmessageType_t type, uint8_t * payload, size_t length) {
            this->webSocketEvent(type, payload, length);
    });

    return true;
}

bool ServerComms::sendEvent(const String& event, const String& payload)
{
    // Format:  ["<event>","<payload>"]
    bool bRetVal = true;
    char sBuf[2048];

    sprintf(sBuf, "[\"%s\",\"%s\"]", event.c_str(), payload.c_str());
    String sEvent = sBuf;
    
    if (m_WSC.sendEVENT(sBuf)) {
        Serial.printf("Event: %s Sent successfully!\n",sEvent.c_str());
    }
    else {
        Serial.printf("Event: %s Failed to send!\n",sEvent.c_str());
        bRetVal = false;
    }

    return bRetVal;
}
