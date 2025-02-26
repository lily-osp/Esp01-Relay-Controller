#include "device.h"

void setDeviceState(bool isOn) {
  deviceState = isOn;
  for (int i = 0; i < config.relayCount; i++) {
    digitalWrite(config.relayPins[i], isOn ? LOW : HIGH); // Toggle relay state
  }

  startLedPattern(isOn ? LED_PATTERN_ACTIVE : LED_PATTERN_IDLE);

  saveDeviceState(isOn);
  broadcastStatus(isOn);

  if (config.useAdafruitIO && relayFeed) {
    relayFeed->save(isOn ? 1 : 0);
  }
}

void saveDeviceState(bool isOn) {
  EEPROM.begin(sizeof(DeviceConfig) + config.relayCount); // Reserve space for relay states
  EEPROM.put(CONFIG_ADDRESS, config); // Save device configuration
  for (int i = 0; i < config.relayCount; i++) {
    bool relayState = digitalRead(config.relayPins[i]) == LOW; // Get current relay state
    EEPROM.put(CONFIG_ADDRESS + sizeof(DeviceConfig) + i, relayState); // Save relay state
  }
  EEPROM.commit();
  EEPROM.end();
}

void loadDeviceState() {
  EEPROM.begin(sizeof(DeviceConfig) + config.relayCount); // Reserve space for relay states
  EEPROM.get(CONFIG_ADDRESS, config); // Load device configuration
  
  // First relay state will determine the overall device state
  bool firstRelayState;
  EEPROM.get(CONFIG_ADDRESS + sizeof(DeviceConfig), firstRelayState);
  deviceState = firstRelayState;
  
  for (int i = 0; i < config.relayCount; i++) {
    bool relayState;
    EEPROM.get(CONFIG_ADDRESS + sizeof(DeviceConfig) + i, relayState); // Load relay state
    digitalWrite(config.relayPins[i], relayState ? LOW : HIGH); // Set relay state
  }
  EEPROM.end();
  
  // Update LED pattern based on restored state
  startLedPattern(deviceState ? LED_PATTERN_ACTIVE : LED_PATTERN_IDLE);
}

void broadcastStatus(bool isOn) {
  String status = isOn ? "ON" : "OFF";
  String message = "{\"type\":\"status\",\"state\":\"" + status + "\"}";
  webSocket.broadcastTXT(message);
}