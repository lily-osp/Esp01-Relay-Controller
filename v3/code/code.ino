#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WebSocketsServer.h>
#include <EEPROM.h>
#include <DNSServer.h>
#include <AdafruitIO_WiFi.h>
#include <DHT.h>
#include "UI.h"

// Constants
#define LED_BLINK_INTERVAL 200
#define LED_PATTERN_PAUSE 1000
#define QUICK_BLINK_INTERVAL 100
#define SLOW_BLINK_INTERVAL 500
#define ERROR_BLINK_INTERVAL 150

#define SETUP_MODE_PATTERN 3
#define SUCCESS_PATTERN 4
#define ERROR_PATTERN 5
#define WIFI_CONNECTING_PATTERN 2
#define IO_CONNECTING_PATTERN 3
#define RESET_PATTERN 10

#define RELAY_PIN 0
#define LED_PIN 1
#define CONFIG_ADDRESS 0
#define AP_SSID "ESP8266-Setup"
#define AP_PASSWORD "configme123"
const byte DNS_PORT = 53;

#define WIFI_CONNECT_TIMEOUT 15000
#define WIFI_RETRY_INTERVAL 30000
#define MAX_WIFI_CONNECT_ATTEMPTS 5

#define MAX_RELAYS 4
#define MAX_SENSORS 6
#define SENSOR_UPDATE_INTERVAL 5000
#define IP_UPDATE_INTERVAL 5 * 60 * 1000 // 5 minutes

// Sensor Types
enum SensorType {
  SENSOR_NONE,
  SENSOR_DHT,
  SENSOR_SOIL,
  SENSOR_WATER,
  SENSOR_LDR,
  SENSOR_LM35
};

// Sensor Configuration
struct SensorConfig {
  SensorType type;
  uint8_t pin;
  uint8_t dhtType; // Only for DHT (11 or 22)
};

// Device Configuration
struct DeviceConfig {
  char wifiSSID[32];
  char wifiPassword[64];
  char mdnsName[32];
  bool useAdafruitIO;
  char ioUsername[32];
  char ioKey[64];
  char relayFeedName[32];
  char ipFeedName[32];
  uint8_t configVersion;
  bool savedDeviceState;
  uint8_t relayPins[MAX_RELAYS];
  SensorConfig sensors[MAX_SENSORS];
  uint8_t relayCount;
  uint8_t sensorCount;
};

// Global Variables
DeviceConfig config;
ESP8266WebServer server(80);
DNSServer dnsServer;
WebSocketsServer webSocket(81);
AdafruitIO_WiFi *io = nullptr;
AdafruitIO_Feed *relayFeed = nullptr;
AdafruitIO_Feed *ipFeed = nullptr;

bool isConfigMode = false;
unsigned long wifiConnectStartTime = 0;
int wifiConnectAttempts = 0;

unsigned long lastLedUpdate = 0;
bool ledState = false;
int currentBlink = 0;
bool isPatternActive = false;
int currentPattern = 0;
unsigned long currentBlinkInterval = LED_BLINK_INTERVAL;
bool patternRepeating = false;

unsigned long lastSensorUpdate = 0;
unsigned long lastIPUpdateTime = 0;
DHT* dhtSensors[MAX_SENSORS];

// Declare deviceState as a global variable
bool deviceState = false;

// LED Patterns
enum LedPattern {
  PATTERN_NONE = 0,
  PATTERN_SETUP_MODE = 1,
  PATTERN_SUCCESS = 2,
  PATTERN_ERROR = 3,
  PATTERN_WIFI_CONNECTING = 4,
  PATTERN_IO_CONNECTING = 5,
  PATTERN_RESET = 6,
  PATTERN_ACTIVE = 7,
  PATTERN_IDLE = 8
};

// Function Prototypes
void updateLedPattern();
void handleRepeatingPattern(int blinkCount);
void handleSinglePattern(int blinkCount);
void handleBreathingEffect();
void startLedPattern(LedPattern pattern);
void saveConfig();
void loadConfig();
void startCaptivePortal();
bool attemptWiFiConnection();
void checkWiFiConnection();
void setupMDNS();
void setupAdafruitIO();
void sendIPToAdafruitIO();
void handleSetup();
void handleNotFound();
void handleSetupMode();
void handleSaveConfig();
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
void setDeviceState(bool isOn);
void saveDeviceState(bool isOn);
void broadcastStatus(bool isOn);
void handleRelayFeed(AdafruitIO_Data *data);
void initializeSensors();
void readSensors();

// LED Pattern Handling
void updateLedPattern() {
  unsigned long currentMillis = millis();

  if (!isPatternActive) return;

  if (currentMillis - lastLedUpdate >= currentBlinkInterval) {
    lastLedUpdate = currentMillis;

    switch (currentPattern) {
      case PATTERN_SETUP_MODE:
        currentBlinkInterval = QUICK_BLINK_INTERVAL;
        patternRepeating = true;
        handleRepeatingPattern(SETUP_MODE_PATTERN);
        break;

      case PATTERN_SUCCESS:
        currentBlinkInterval = LED_BLINK_INTERVAL;
        patternRepeating = false;
        handleSinglePattern(SUCCESS_PATTERN);
        break;

      case PATTERN_ERROR:
        currentBlinkInterval = ERROR_BLINK_INTERVAL;
        patternRepeating = false;
        handleSinglePattern(ERROR_PATTERN);
        break;

      case PATTERN_WIFI_CONNECTING:
        currentBlinkInterval = SLOW_BLINK_INTERVAL;
        patternRepeating = true;
        handleRepeatingPattern(WIFI_CONNECTING_PATTERN);
        break;

      case PATTERN_IO_CONNECTING:
        currentBlinkInterval = LED_BLINK_INTERVAL;
        patternRepeating = true;
        handleRepeatingPattern(IO_CONNECTING_PATTERN);
        break;

      case PATTERN_RESET:
        currentBlinkInterval = QUICK_BLINK_INTERVAL;
        patternRepeating = false;
        handleSinglePattern(RESET_PATTERN);
        break;

      case PATTERN_ACTIVE:
        digitalWrite(LED_PIN, LOW);
        break;

      case PATTERN_IDLE:
        handleBreathingEffect();
        break;
    }
  }
}

void handleRepeatingPattern(int blinkCount) {
  ledState = !ledState;
  digitalWrite(LED_PIN, ledState ? LOW : HIGH);

  if (!ledState) {
    currentBlink++;
    if (currentBlink >= blinkCount * 2) {
      currentBlink = 0;
      delay(LED_PATTERN_PAUSE);
    }
  }
}

void handleSinglePattern(int blinkCount) {
  ledState = !ledState;
  digitalWrite(LED_PIN, ledState ? LOW : HIGH);

  if (!ledState) {
    currentBlink++;
    if (currentBlink >= blinkCount * 2) {
      isPatternActive = false;
      currentPattern = PATTERN_IDLE;
      currentBlink = 0;
      digitalWrite(LED_PIN, HIGH);
    }
  }
}

void handleBreathingEffect() {
  static int brightness = 0;
  static bool increasing = true;

  if (increasing) {
    brightness++;
    if (brightness >= 255) increasing = false;
  } else {
    brightness--;
    if (brightness <= 0) increasing = true;
  }

  analogWrite(LED_PIN, 255 - brightness);
}

void startLedPattern(LedPattern pattern) {
  currentPattern = pattern;
  currentBlink = 0;
  ledState = false;
  isPatternActive = true;
  lastLedUpdate = millis();

  switch (pattern) {
    case PATTERN_SETUP_MODE:
      currentBlinkInterval = QUICK_BLINK_INTERVAL;
      break;
    case PATTERN_ERROR:
      currentBlinkInterval = ERROR_BLINK_INTERVAL;
      break;
    case PATTERN_WIFI_CONNECTING:
      currentBlinkInterval = SLOW_BLINK_INTERVAL;
      break;
    default:
      currentBlinkInterval = LED_BLINK_INTERVAL;
  }

  digitalWrite(LED_PIN, HIGH);
}

// Configuration Handling
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

// WiFi and Setup Mode
void startCaptivePortal() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  IPAddress apIP = WiFi.softAPIP();
  dnsServer.start(DNS_PORT, "*", apIP);

  server.on("/", handleSetup);
  server.on("/setup", handleSetup);
  server.on("/save-config", HTTP_POST, handleSaveConfig);
  server.on("/enter-setup", handleSetupMode);
  server.onNotFound(handleNotFound);
  server.begin();

  isConfigMode = true;
  startLedPattern(PATTERN_SETUP_MODE);
}

bool attemptWiFiConnection() {
  if (strlen(config.wifiSSID) == 0) {
    startLedPattern(PATTERN_ERROR);
    return false;
  }

  startLedPattern(PATTERN_WIFI_CONNECTING);

  WiFi.mode(WIFI_STA);
  WiFi.begin(config.wifiSSID, config.wifiPassword);

  wifiConnectStartTime = millis();

  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - wifiConnectStartTime > WIFI_CONNECT_TIMEOUT) {
      startLedPattern(PATTERN_ERROR);
      return false;
    }
    delay(500);
  }

  startLedPattern(PATTERN_SUCCESS);
  return true;
}

void checkWiFiConnection() {
  if (isConfigMode) return;

  if (WiFi.status() != WL_CONNECTED) {
    wifiConnectAttempts++;

    if (wifiConnectAttempts >= MAX_WIFI_CONNECT_ATTEMPTS) {
      startLedPattern(PATTERN_ERROR);
      startCaptivePortal();
      return;
    }

    startLedPattern(PATTERN_WIFI_CONNECTING);
    if (!attemptWiFiConnection()) {
      delay(WIFI_RETRY_INTERVAL);
    }
  } else {
    wifiConnectAttempts = 0;
  }
}

// mDNS and Adafruit IO
void setupMDNS() {
  if (MDNS.begin(config.mdnsName)) {
    startLedPattern(PATTERN_SUCCESS);
  } else {
    startLedPattern(PATTERN_ERROR);
  }
}

void setupAdafruitIO() {
  if (config.useAdafruitIO) {
    if (strlen(config.ioUsername) == 0 || strlen(config.ioKey) == 0) {
      startLedPattern(PATTERN_ERROR);
      return;
    }

    if (io) delete io;
    if (relayFeed) delete relayFeed;
    if (ipFeed) delete ipFeed;

    startLedPattern(PATTERN_IO_CONNECTING);

    io = new AdafruitIO_WiFi(
      config.ioUsername,
      config.ioKey,
      config.wifiSSID,
      config.wifiPassword
    );

    relayFeed = io->feed(config.relayFeedName);
    ipFeed = io->feed(config.ipFeedName);

    if (relayFeed) {
      relayFeed->onMessage(handleRelayFeed);
    }

    unsigned long connectStartTime = millis();
    while (io->status() < AIO_CONNECTED) {
      io->run();

      if (millis() - connectStartTime > 10000) {
        startLedPattern(PATTERN_ERROR);
        delete io;
        io = nullptr;
        relayFeed = nullptr;
        ipFeed = nullptr;
        return;
      }
      delay(500);
    }

    startLedPattern(PATTERN_SUCCESS);
  }
}

void sendIPToAdafruitIO() {
  if (config.useAdafruitIO && io && io->status() >= AIO_CONNECTED && ipFeed) {
    if (WiFi.status() == WL_CONNECTED) {
      String ipAddress = WiFi.localIP().toString();
      ipFeed->save(ipAddress);
    }
  }
}

// Web Server Handlers
void handleSetup() {
  server.send_P(200, "text/html", SETUP_UI);
}

void handleNotFound() {
  server.sendHeader("Location", String("/"), true);
  server.send(302, "text/plain", "");
}

void handleSetupMode() {
  server.send(200, "text/plain", "Entering setup mode...");
  delay(1000);
  ESP.restart();
}

void handleSaveConfig() {
  strncpy(config.wifiSSID, server.arg("wifiSSID").c_str(), sizeof(config.wifiSSID) - 1);
  strncpy(config.wifiPassword, server.arg("wifiPassword").c_str(), sizeof(config.wifiPassword) - 1);
  strncpy(config.mdnsName, server.arg("mdnsName").c_str(), sizeof(config.mdnsName) - 1);

  config.useAdafruitIO = (server.arg("useAdafruitIO") == "true");

  if (config.useAdafruitIO) {
    strncpy(config.ioUsername, server.arg("ioUsername").c_str(), sizeof(config.ioUsername) - 1);
    strncpy(config.ioKey, server.arg("ioKey").c_str(), sizeof(config.ioKey) - 1);
    strncpy(config.relayFeedName, server.arg("relayFeedName").c_str(), sizeof(config.relayFeedName) - 1);
    strncpy(config.ipFeedName, server.arg("ipFeedName").c_str(), sizeof(config.ipFeedName) - 1);
  }

  config.relayCount = server.arg("relayCount").toInt();
  for (int i = 0; i < config.relayCount; i++) {
    config.relayPins[i] = server.arg("relayPin" + String(i)).toInt();
  }

  config.sensorCount = 0;
  for (int i = 0; i < MAX_SENSORS; i++) {
    String sensorType = server.arg("sensor" + String(i) + "_type");
    if (sensorType.length() > 0) {
      config.sensors[config.sensorCount].type = static_cast<SensorType>(sensorType.toInt());
      config.sensors[config.sensorCount].pin = server.arg("sensor" + String(i) + "_pin").toInt();
      if (config.sensors[config.sensorCount].type == SENSOR_DHT) {
        config.sensors[config.sensorCount].dhtType = server.arg("sensor" + String(i) + "_dhtType").toInt();
      }
      config.sensorCount++;
    }
  }

  saveConfig();
  startLedPattern(PATTERN_RESET);

  server.send(200, "text/plain", "Configuration saved! Rebooting...");
  delay(1000);
  ESP.restart();
}

// WebSocket Event Handler
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_CONNECTED:
      {
        String status = deviceState ? "ON" : "OFF";
        webSocket.sendTXT(num, status);
      }
      break;
    case WStype_TEXT:
      {
        String command = String((char *)payload);
        if (command == "ON") {
          setDeviceState(true);
        } else if (command == "OFF") {
          setDeviceState(false);
        }
      }
      break;
  }
}

// Device State Management
void setDeviceState(bool isOn) {
  deviceState = isOn;
  for (int i = 0; i < config.relayCount; i++) {
    digitalWrite(config.relayPins[i], isOn ? LOW : HIGH);
  }

  startLedPattern(isOn ? PATTERN_ACTIVE : PATTERN_IDLE);

  saveDeviceState(isOn);
  broadcastStatus(isOn);

  if (config.useAdafruitIO && relayFeed) {
    relayFeed->save(isOn ? 1 : 0);
  }
}

void saveDeviceState(bool isOn) {
  EEPROM.begin(4);
  uint8_t state = isOn ? 1 : 0;
  EEPROM.write(CONFIG_ADDRESS + sizeof(DeviceConfig), state);
  EEPROM.commit();
}

void broadcastStatus(bool isOn) {
  String status = isOn ? "ON" : "OFF";
  webSocket.broadcastTXT(status);
}

void handleRelayFeed(AdafruitIO_Data *data) {
  if (!data) return;

  int relayValue = data->toInt();
  bool newState = (relayValue == 1);

  if (newState != deviceState) {
    setDeviceState(newState);
  }
}

// Sensor Initialization and Reading
void initializeSensors() {
  for (int i = 0; i < config.sensorCount; i++) {
    if (config.sensors[i].type == SENSOR_DHT) {
      dhtSensors[i] = new DHT(config.sensors[i].pin, config.sensors[i].dhtType);
      dhtSensors[i]->begin();
    }
    // Initialize other sensor types as needed
  }
}

void readSensors() {
  if (millis() - lastSensorUpdate > SENSOR_UPDATE_INTERVAL) {
    for (int i = 0; i < config.sensorCount; i++) {
      switch(config.sensors[i].type) {
        case SENSOR_DHT:
          if (dhtSensors[i]) {
            float h = dhtSensors[i]->readHumidity();
            float t = dhtSensors[i]->readTemperature();
            // Send via WebSocket
          }
          break;
        case SENSOR_SOIL:
          // Read analog pin
          break;
        // ... handle other sensor types ...
      }
    }
    lastSensorUpdate = millis();
  }
}

// Setup and Loop
void setup() {
  pinMode(LED_PIN, OUTPUT);

  loadConfig();

  // Initialize relay pins
  for (int i = 0; i < config.relayCount; i++) {
    pinMode(config.relayPins[i], OUTPUT);
    digitalWrite(config.relayPins[i], HIGH); // Turn off relays initially
  }

  // Initialize sensors
  initializeSensors();

  WiFi.persistent(true);
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);

  bool wifiConnected = attemptWiFiConnection();

  if (!wifiConnected) {
    startCaptivePortal();
    return;
  }

  setupMDNS();

  if (config.useAdafruitIO) {
    setupAdafruitIO();
  }

  server.on("/", []() {
    server.send_P(200, "text/html", WEB_UI);
  });

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  server.begin();

  setDeviceState(false);
}

void loop() {
  if (isConfigMode) {
    dnsServer.processNextRequest();
  }

  server.handleClient();

  if (!isConfigMode) {
    checkWiFiConnection();
    webSocket.loop();

    if (config.useAdafruitIO) {
      io->run();

      if (millis() - lastIPUpdateTime > IP_UPDATE_INTERVAL) {
        sendIPToAdafruitIO();
        lastIPUpdateTime = millis();
      }
    }

    MDNS.update();

    // Read sensor data
    readSensors();
  }
}