// adafruit_io.cpp
#include "adafruit_io.h"

void setupAdafruitIO()
{
    if (config.useAdafruitIO) {
        if (strlen(config.ioUsername) == 0 || strlen(config.ioKey) == 0) {
            startLedPattern(LED_PATTERN_ERROR);
            return;
        }

        if (io)
            delete io;
        if (relayFeed)
            delete relayFeed;
        if (ipFeed)
            delete ipFeed;

        startLedPattern(LED_PATTERN_IO_CONNECTING);

        io = new AdafruitIO_WiFi(
            config.ioUsername,
            config.ioKey,
            config.wifiSSID,
            config.wifiPassword);

        relayFeed = io->feed(config.relayFeedName);
        ipFeed = io->feed(config.ipFeedName);

        if (relayFeed) {
            relayFeed->onMessage(handleRelayFeed);
        }

        unsigned long connectStartTime = millis();
        while (io->status() < AIO_CONNECTED) {
            io->run();

            if (millis() - connectStartTime > 10000) {
                startLedPattern(LED_PATTERN_ERROR);
                delete io;
                io = nullptr;
                relayFeed = nullptr;
                ipFeed = nullptr;
                return;
            }
            delay(500);
        }

        startLedPattern(LED_PATTERN_SUCCESS);
    }
}

void sendIPToAdafruitIO()
{
    if (config.useAdafruitIO && io && io->status() >= AIO_CONNECTED && ipFeed) {
        if (WiFi.status() == WL_CONNECTED) {
            String ipAddress = WiFi.localIP().toString();
            ipFeed->save(ipAddress);
        }
    }
}

void handleRelayFeed(AdafruitIO_Data* data)
{
    if (!data)
        return;

    int relayValue = data->toInt();
    bool newState = (relayValue == 1);

    if (newState != deviceState) {
        setDeviceState(newState);
    }
}
