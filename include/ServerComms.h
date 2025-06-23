// #include <ArduinoWebsockets.h>
// using namespace websockets;
// #include <SocketIoClient.h>
#include <WebSocketsClient.h>
#include <SocketIOclient.h>

class ServerComms
{
private:
    SocketIOclient m_WSC;
    String m_Token;
    bool m_bConnected;
    bool m_bVersionSent;
    
    ServerComms() { }

    void webSocketEvent(socketIOmessageType_t type, uint8_t * payload, size_t length);
    bool ProcessEvent(char *sEvent);
    void SendVersion();

public:
    static ServerComms * getInstance();

    void SetToken(String Token);
    String GetToken() { return m_Token; }
    bool Connect();
    void Poll();
    bool sendEvent(const String& event, const String& payload);

    bool IsConnected();
};