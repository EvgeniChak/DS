#include "version.h"
#include <Arduino.h>

class BuildVersion 
{
    private:
        static String m_sVersion;

    public:
        BuildVersion();

        String getVersion();
        String getRawJson();
};

