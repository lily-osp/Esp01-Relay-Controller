#ifndef GLOBAL_H
#define GLOBAL_H

#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WebSocketsServer.h>
#include "AdafruitIO_WiFi.h"

// External declarations for global variables
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

#endif // GLOBAL_H
