#include "config.h"
#include "global.h"
#include "led.h"
#include "wifi.h"
#include "device.h"
#include "sensors.h"
#include "webserver.h"
#include "adafruit_io.h"
#include "UI.h"

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  loadConfig();

  // Initialize relay pins
  for (int i = 0; i < config.relayCount; i++) {
    pinMode(config.relayPins[i], OUTPUT);
    digitalWrite(config.relayPins[i], HIGH); // Initialize to OFF state initially
  }

  // Load saved relay states - this will set deviceState and update relays
  loadDeviceState();

  // Initialize sensors
  initializeSensors();

  WiFi.persistent(true);
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);

  bool wifiConnected = attemptWiFiConnection();

  if (!wifiConnected) {
    isSetupMode = true;
    startCaptivePortal();
    return;
  }

  setupMDNS();

  if (config.useAdafruitIO) {
    setupAdafruitIO();
  }

  // Initialize web server and WebSocket server
  initWebServer();

  // Don't call setDeviceState here since loadDeviceState already set everything up
  broadcastStatus(deviceState); // Just broadcast the current state
}

void loop() {
  if (isSetupMode) {
    dnsServer.processNextRequest();
  }

  server.handleClient();

  if (!isSetupMode) {
    checkWiFiConnection();
    webSocket.loop();

    if (config.useAdafruitIO) {
      io->run();

      if (millis() - lastIPUpdate > IP_UPDATE_INTERVAL) {
        sendIPToAdafruitIO();
        lastIPUpdate = millis();
      }
    }

    MDNS.update();

    // Read sensor data if it's time
    if (millis() - lastSensorUpdate > SENSOR_UPDATE_INTERVAL) {
      readSensors();
      lastSensorUpdate = millis();
    }
  }
}