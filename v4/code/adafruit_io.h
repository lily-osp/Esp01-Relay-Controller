// adafruit_io.h
#ifndef ADAFRUIT_IO_H
#define ADAFRUIT_IO_H

#include "config.h"
#include "led.h"
#include "device.h"
#include <AdafruitIO_WiFi.h>

void setupAdafruitIO();
void sendIPToAdafruitIO();
void handleRelayFeed(AdafruitIO_Data* data);

#endif
