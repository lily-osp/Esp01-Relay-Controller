// device.h
#ifndef DEVICE_H
#define DEVICE_H

#include "config.h"

void setDeviceState(bool isOn);
void saveDeviceState(bool isOn);
void loadDeviceState();
void broadcastStatus(bool isOn);

#endif
