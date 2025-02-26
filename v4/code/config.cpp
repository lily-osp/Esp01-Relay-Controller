// config.cpp
#include "config.h"
#include "global.h"

// Global variable definitions
DeviceConfig config;
ESP8266WebServer server(80);
DNSServer dnsServer;
WebSocketsServer webSocket(81);
AdafruitIO_WiFi* io = nullptr;
AdafruitIO_Feed* relayFeed = nullptr;
AdafruitIO_Feed* ipFeed = nullptr;
bool deviceState = false;
bool isSetupMode = false;
unsigned long lastSensorUpdate = 0;
unsigned long lastIPUpdate = 0;

void saveConfig() {
  config.savedDeviceState = deviceState;
  config.configVersion = 42;
  EEPROM.begin(sizeof(DeviceConfig));
  EEPROM.put(CONFIG_ADDRESS, config);
  EEPROM.commit();
}

void loadConfig() {
  EEPROM.begin(sizeof(DeviceConfig));
  EEPROM.get(CONFIG_ADDRESS, config);

  if (config.configVersion != 42) {
    memset(&config, 0, sizeof(DeviceConfig));
    strcpy(config.mdnsName, "esp-device");
    strcpy(config.relayFeedName, "relay");
    strcpy(config.ipFeedName, "ip");
    config.useAdafruitIO = false;
    config.savedDeviceState = false;
    config.relayCount = 0;
    config.sensorCount = 0;
    saveConfig();
  }

  setDeviceState(config.savedDeviceState);
}