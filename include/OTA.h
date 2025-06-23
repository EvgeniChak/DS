#include <WiFi.h>
#include <HTTPClient.h>
#include <FS.h>
#include <SPIFFS.h>

class OTA 
{
    public:
        OTA(String sFile);

        bool Start();

    private:
        bool DoDownload();
        bool DoUpdate();

    private:
        String m_sFile;

};
