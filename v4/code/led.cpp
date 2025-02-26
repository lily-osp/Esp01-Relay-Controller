// led.cpp
#include "led.h"

unsigned long lastLedUpdate = 0;
bool ledState = false;
int currentBlink = 0;
bool isPatternActive = false;
int currentPattern = LED_PATTERN_NONE;
unsigned long currentBlinkInterval = LED_BLINK_INTERVAL;
bool patternRepeating = false;

void updateLedPattern()
{
    unsigned long currentMillis = millis();

    if (!isPatternActive)
        return;

    if (currentMillis - lastLedUpdate >= currentBlinkInterval) {
        lastLedUpdate = currentMillis;

        switch (currentPattern) {
        case LED_PATTERN_SETUP:
            currentBlinkInterval = QUICK_BLINK_INTERVAL;
            patternRepeating = true;
            handleRepeatingPattern(SETUP_MODE_PATTERN);
            break;

        case LED_PATTERN_SUCCESS:
            currentBlinkInterval = LED_BLINK_INTERVAL;
            patternRepeating = false;
            handleSinglePattern(SUCCESS_PATTERN);
            break;

        case LED_PATTERN_ERROR:
            currentBlinkInterval = ERROR_BLINK_INTERVAL;
            patternRepeating = false;
            handleSinglePattern(ERROR_PATTERN);
            break;

        case LED_PATTERN_WIFI_CONNECTING:
            currentBlinkInterval = SLOW_BLINK_INTERVAL;
            patternRepeating = true;
            handleRepeatingPattern(WIFI_CONNECTING_PATTERN);
            break;

        case LED_PATTERN_IO_CONNECTING:
            currentBlinkInterval = LED_BLINK_INTERVAL;
            patternRepeating = true;
            handleRepeatingPattern(IO_CONNECTING_PATTERN);
            break;

        case LED_PATTERN_RESET:
            currentBlinkInterval = QUICK_BLINK_INTERVAL;
            patternRepeating = false;
            handleSinglePattern(ERROR_PATTERN);
            break;

        case LED_PATTERN_ACTIVE:
            digitalWrite(LED_PIN, LOW);
            break;

        case LED_PATTERN_IDLE:
            handleBreathingEffect();
            break;
        }
    }
}

void handleRepeatingPattern(int blinkCount)
{
    digitalWrite(LED_PIN, ledState ? LOW : HIGH);
    ledState = !ledState;
    currentBlink++;

    if (currentBlink >= blinkCount * 2) {
        currentBlink = 0;
        if (!patternRepeating) {
            isPatternActive = false;
        }
    }
}

void handleSinglePattern(int blinkCount)
{
    digitalWrite(LED_PIN, ledState ? LOW : HIGH);
    ledState = !ledState;
    currentBlink++;

    if (currentBlink >= blinkCount * 2) {
        currentBlink = 0;
        isPatternActive = false;
        currentPattern = LED_PATTERN_IDLE;
    }
}

void handleBreathingEffect()
{
    static int brightness = 0;
    static int fadeAmount = 5;
    static unsigned long lastBreathUpdate = 0;
    unsigned long currentMillis = millis();

    if (currentMillis - lastBreathUpdate >= 30) {
        lastBreathUpdate = currentMillis;
        brightness = brightness + fadeAmount;
        if (brightness <= 0 || brightness >= 255) {
            fadeAmount = -fadeAmount;
        }
        analogWrite(LED_PIN, 255 - brightness);
    }
}

void startLedPattern(LedPattern pattern)
{
    currentBlink = 0;
    ledState = false;
    isPatternActive = true;
    currentPattern = pattern;

    switch (pattern) {
        case LED_PATTERN_SETUP:
        case LED_PATTERN_ERROR:
        case LED_PATTERN_WIFI_CONNECTING:
        case LED_PATTERN_IO_CONNECTING:
            digitalWrite(LED_PIN, HIGH);
            break;
        default:
            digitalWrite(LED_PIN, HIGH);
            break;
    }
}