// webserver.h
#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "config.h"

void initWebServer();
void handleSetup();
void handleSetupMode();
void handleNotFound();
void handleSaveConfig();

extern WebSocketsServer webSocket;

#endif
