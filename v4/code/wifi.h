// wifi.h
#ifndef WIFI_H
#define WIFI_H

#include "config.h"

void startCaptivePortal();
bool attemptWiFiConnection();
void checkWiFiConnection();
void setupMDNS();

#endif
