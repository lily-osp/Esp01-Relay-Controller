#include "config.h"
#include "global.h"

// External declarations - these variables are defined in config.cpp
extern DeviceConfig config;
extern ESP8266WebServer server;
extern DNSServer dnsServer;
extern WebSocketsServer webSocket;
extern AdafruitIO_WiFi* io;
extern AdafruitIO_Feed* relayFeed;
extern AdafruitIO_Feed* ipFeed;
extern bool deviceState;
extern bool isSetupMode;
extern unsigned long lastSensorUpdate;
extern unsigned long lastIPUpdate;
