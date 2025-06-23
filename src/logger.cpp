#include "logger.h"

void initLogger() {
    DEBUG_PORT.begin(115200);
    while (!DEBUG_PORT) {
        ;
    }
    DEBUG_INFO(F("Logger initialized"));
}
