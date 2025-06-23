
#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

enum State {
    INIT,           // Initialize the system, read JSON files
    WIFI_CONNECT,   // Connect to WiFi, handle failures
    ACTIVATION,     // Use activation code to obtain certificates if necessary
    MQTT_CONNECT,   // Establish AWS IOT connection over TLS
    MQTT_PUBLISH,   // Periodically publish MQTT keepalive messages
    WEBSOCKET_CONNECT, // Connect through Web Sockets
    WIFI_MANAGER    // Handle WiFi management (future implementation)
};

extern State currentState;


void runStateMachine();

#endif // STATE_MACHINE_H
