#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>

#define DEBUG_PORT Serial
#define DEBUG_ENABLE

#ifdef DEBUG_ENABLE
#define DEBUG(msg) DEBUG_PORT.println(msg)
#define DEBUG_F(msg, ...)                               \
    do {                                                \
        char _buf[100];                                 \
        snprintf(_buf, sizeof(_buf), msg, __VA_ARGS__); \
        DEBUG_PORT.println(_buf);                       \
    } while (0)
#define DEBUG_WARNING(msg)             \
    do {                               \
        DEBUG_PORT.print("WARNING: "); \
        DEBUG_PORT.println(msg);       \
    } while (0)
#define DEBUG_INFO(msg)             \
    do {                            \
        DEBUG_PORT.print("INFO: "); \
        DEBUG_PORT.println(msg);    \
    } while (0)
#define DEBUG_ERROR(msg)             \
    do {                             \
        DEBUG_PORT.print("ERROR: "); \
        DEBUG_PORT.println(msg);     \
    } while (0)
#define DEBUG_ERROR_CODE(t, c, m)    \
    do {                             \
        DEBUG_PORT.print("ERROR: "); \
        DEBUG_PORT.print(t);         \
        DEBUG_PORT.print(" (code "); \
        DEBUG_PORT.print(c);         \
        DEBUG_PORT.print("): ");     \
        DEBUG_PORT.println(m);       \
    } while (0)
#else
#define DEBUG(msg)
#define DEBUG_F(msg, ...)
#define DEBUG_WARNING(msg)
#define DEBUG_INFO(msg)
#define DEBUG_ERROR(msg)
#define DEBUG_ERROR_CODE(type, code, msg)
#endif

void initLogger();
#endif
