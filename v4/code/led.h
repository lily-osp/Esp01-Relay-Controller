// led.h
#ifndef LED_H
#define LED_H

#include <Arduino.h>
#include "config.h"

// Define the LedPattern enum
enum LedPattern {
    LED_PATTERN_NONE = 0,
    LED_PATTERN_SETUP = 1,
    LED_PATTERN_SUCCESS = 2,
    LED_PATTERN_ERROR = 3,
    LED_PATTERN_WIFI_CONNECTING = 4,
    LED_PATTERN_IO_CONNECTING = 5,
    LED_PATTERN_RESET = 6,
    LED_PATTERN_ACTIVE = 7,
    LED_PATTERN_IDLE = 8
};

// LED timing constants
#define LED_BLINK_INTERVAL 200
#define LED_PATTERN_PAUSE 1000
#define QUICK_BLINK_INTERVAL 100
#define SLOW_BLINK_INTERVAL 500
#define ERROR_BLINK_INTERVAL 150

// LED pattern blink counts
#define SETUP_MODE_PATTERN 2
#define SUCCESS_PATTERN 3
#define ERROR_PATTERN 5
#define WIFI_CONNECTING_PATTERN 2
#define IO_CONNECTING_PATTERN 3

void startLedPattern(LedPattern pattern);
void updateLedPattern();
void handleRepeatingPattern(int blinkCount);
void handleSinglePattern(int blinkCount);
void handleBreathingEffect();

#endif