#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WebSocketsServer.h>
#include <EEPROM.h>
#include <DNSServer.h>
#include <AdafruitIO_WiFi.h>
#include "UI.h"  // Includes setup UI and main UI

// Configuration Structure
struct DeviceConfig {
  char wifiSSID[32];
  char wifiPassword[64];
  char mdnsName[32];
  bool useAdafruitIO;
  char ioUsername[32];
  char ioKey[64];
  char relayFeedName[32];
  char ipFeedName[32];
  uint8_t configVersion;  // To validate config
  bool savedDeviceState;  // Store last known device state
};

// Pins
#define RELAY_PIN 0
#define LED_PIN 1

// EEPROM Addresses
#define CONFIG_ADDRESS 0

// Captive Portal Configuration
#define AP_SSID "ESP8266-Setup"
#define AP_PASSWORD "configme123"
const byte DNS_PORT = 53;

// WiFi Connection Parameters
#define WIFI_CONNECT_TIMEOUT 15000     // 15 seconds connection attempt
#define WIFI_RETRY_INTERVAL 30000      // 30 seconds between retry attempts
#define MAX_WIFI_CONNECT_ATTEMPTS 5    // Maximum connection attempts before setup mode

// Periodic IP update variables
unsigned long lastIPUpdateTime = 0;
const unsigned long IP_UPDATE_INTERVAL = 5 * 60 * 1000; // 5 minutes

// Global Variables
bool deviceState = false;
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

// Function Prototypes
void saveConfig();
void loadConfig();
void startCaptivePortal();
void setupAdafruitIO();
void handleSetup();
void handleSaveConfig();
void handleNotFound();
void checkWiFiConnection();

// Save Configuration to EEPROM
void saveConfig() {
  // Save current device state in config
  config.savedDeviceState = deviceState;

  config.configVersion = 42;  // Magic number to validate config
  EEPROM.begin(sizeof(DeviceConfig));
  EEPROM.put(CONFIG_ADDRESS, config);
  EEPROM.commit();
}

// Load Configuration from EEPROM
void loadConfig() {
  EEPROM.begin(sizeof(DeviceConfig));
  EEPROM.get(CONFIG_ADDRESS, config);

  // Check if config is valid
  if (config.configVersion != 42) {
    // Set default values
    memset(&config, 0, sizeof(DeviceConfig));
    strcpy(config.mdnsName, "esp-device");
    strcpy(config.relayFeedName, "relay");
    strcpy(config.ipFeedName, "ip");
    config.useAdafruitIO = false;
    config.savedDeviceState = false;  // Default to OFF
    saveConfig();
  }

  // Set initial device state from saved config
  setDeviceState(config.savedDeviceState);
}

// Start Captive Portal
void startCaptivePortal() {
  Serial.println("Starting Captive Portal");

  // Disable WiFi Station mode
  WiFi.mode(WIFI_AP);

  // Create Access Point
  WiFi.softAP(AP_SSID, AP_PASSWORD);

  // Get AP IP
  IPAddress apIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(apIP);

  // Start DNS server for captive portal
  dnsServer.start(DNS_PORT, "*", apIP);

  // Setup server routes
  server.on("/", handleSetup);
  server.on("/setup", handleSetup);
  server.on("/save-config", HTTP_POST, handleSaveConfig);
  server.onNotFound(handleNotFound);
  server.begin();

  isConfigMode = true;
  Serial.println("Captive Portal Started. Connect to WiFi: ESP8266-Setup");
}

// Attempt WiFi Connection
bool attemptWiFiConnection() {
  // Check if WiFi credentials are set
  if (strlen(config.wifiSSID) == 0) {
    Serial.println("No WiFi credentials set");
    return false;
  }

  Serial.print("Attempting to connect to WiFi: ");
  Serial.println(config.wifiSSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(config.wifiSSID, config.wifiPassword);

  // Record start time of connection attempt
  wifiConnectStartTime = millis();

  // Wait for connection with timeout
  while (WiFi.status() != WL_CONNECTED) {
    // Check connection timeout
    if (millis() - wifiConnectStartTime > WIFI_CONNECT_TIMEOUT) {
      Serial.println("\nWiFi connection timeout");
      return false;
    }

    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  return true;
}

// Check and manage WiFi connection
void checkWiFiConnection() {
  // If already in config mode, do nothing
  if (isConfigMode) return;

  // Check WiFi status
  if (WiFi.status() != WL_CONNECTED) {
    // Increment connection attempts
    wifiConnectAttempts++;

    Serial.print("WiFi connection lost. Attempt: ");
    Serial.println(wifiConnectAttempts);

    // If max attempts reached, start setup mode
    if (wifiConnectAttempts >= MAX_WIFI_CONNECT_ATTEMPTS) {
      Serial.println("Max WiFi connection attempts reached. Entering setup mode.");
      startCaptivePortal();
      return;
    }

    // Try to reconnect
    if (!attemptWiFiConnection()) {
      // Wait before next attempt
      delay(WIFI_RETRY_INTERVAL);
    }
  } else {
    // Reset connection attempts if successfully connected
    wifiConnectAttempts = 0;
  }
}

// Setup mDNS
void setupMDNS() {
  if (MDNS.begin(config.mdnsName)) {
    Serial.print("mDNS responder started: ");
    Serial.print(config.mdnsName);
    Serial.println(".local");
  } else {
    Serial.println("Error setting up mDNS responder!");
  }
}

// Setup Adafruit IO if enabled
void setupAdafruitIO() {
  if (config.useAdafruitIO) {
    // Validate configuration
    if (strlen(config.ioUsername) == 0 || strlen(config.ioKey) == 0) {
      Serial.println("Adafruit IO credentials not configured!");
      return;
    }

    // Clean up previous connection if exists
    if (io) delete io;
    if (relayFeed) delete relayFeed;
    if (ipFeed) delete ipFeed;

    Serial.println("Connecting to Adafruit IO...");
    Serial.print("Username: ");
    Serial.println(config.ioUsername);
    Serial.print("Relay Feed: ");
    Serial.println(config.relayFeedName);
    Serial.print("IP Feed: ");
    Serial.println(config.ipFeedName);

    // Create new Adafruit IO instance
    io = new AdafruitIO_WiFi(
      config.ioUsername,
      config.ioKey,
      config.wifiSSID,
      config.wifiPassword
    );

    // Create feeds
    relayFeed = io->feed(config.relayFeedName);
    ipFeed = io->feed(config.ipFeedName);

    // Set up feed callback
    if (relayFeed) {
      relayFeed->onMessage(handleRelayFeed);
    }

    // Attempt connection with timeout
    unsigned long connectStartTime = millis();
    while (io->status() < AIO_CONNECTED) {
      io->run();

      // Print connection status periodically
      if (millis() - connectStartTime > 10000) {
        Serial.println("Adafruit IO connection timeout!");
        break;
      }

      Serial.print(".");
      delay(500);
    }

    // Check final connection status
    if (io->status() >= AIO_CONNECTED) {
      Serial.println("\nAdafruit IO connected successfully!");
    } else {
      Serial.println("\nFailed to connect to Adafruit IO");

      // Clean up failed connection
      delete io;
      io = nullptr;
      relayFeed = nullptr;
      ipFeed = nullptr;
    }
  }
}

// Modify sendIPToAdafruitIO to add more error checking
void sendIPToAdafruitIO() {
  if (config.useAdafruitIO && io && io->status() >= AIO_CONNECTED && ipFeed) {
    if (WiFi.status() == WL_CONNECTED) {
      String ipAddress = WiFi.localIP().toString();

      // Additional debug logging
      Serial.print("Attempting to send IP to Adafruit IO: ");
      Serial.println(ipAddress);

      ipFeed->save(ipAddress);
      Serial.println("IP sent successfully");
    } else {
      Serial.println("Cannot send IP: WiFi not connected");
    }
  } else {
    Serial.println("Cannot send IP: Adafruit IO not configured or connected");
  }
}

// Handle Setup Page
void handleSetup() {
  server.send_P(200, "text/html", SETUP_UI);
}

// Handle 404 Not Found (for captive portal)
void handleNotFound() {
  server.sendHeader("Location", String("/"), true);
  server.send(302, "text/plain", "");
}

// Handle Configuration Save
void handleSaveConfig() {
  // WiFi Configuration
  strncpy(config.wifiSSID, server.arg("wifiSSID").c_str(), sizeof(config.wifiSSID) - 1);
  strncpy(config.wifiPassword, server.arg("wifiPassword").c_str(), sizeof(config.wifiPassword) - 1);

  // mDNS Name
  strncpy(config.mdnsName, server.arg("mdnsName").c_str(), sizeof(config.mdnsName) - 1);

  // Adafruit IO Configuration
  config.useAdafruitIO = (server.arg("useAdafruitIO") == "true");

  if (config.useAdafruitIO) {
    strncpy(config.ioUsername, server.arg("ioUsername").c_str(), sizeof(config.ioUsername) - 1);
    strncpy(config.ioKey, server.arg("ioKey").c_str(), sizeof(config.ioKey) - 1);
    strncpy(config.relayFeedName, server.arg("relayFeedName").c_str(), sizeof(config.relayFeedName) - 1);
    strncpy(config.ipFeedName, server.arg("ipFeedName").c_str(), sizeof(config.ipFeedName) - 1);
  }

  // Save configuration
  saveConfig();

  server.send(200, "text/plain", "Configuration saved! Rebooting...");
  delay(1000);
  ESP.restart();
}

// WebSocket event handler
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_CONNECTED:
      {
        // Send current status when a client connects
        String status = deviceState ? "ON" : "OFF";
        webSocket.sendTXT(num, status);
      }
      break;
    case WStype_TEXT:
      {
        // Handle commands from WebSocket
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

// Function to set device state
void setDeviceState(bool isOn) {
  deviceState = isOn;

  // Active LOW for relay (LOW turns it ON)
  digitalWrite(RELAY_PIN, isOn ? LOW : HIGH);

  // Active LOW for LED (LOW turns it ON)
  digitalWrite(LED_PIN, isOn ? LOW : HIGH);

  // Save state to EEPROM
  saveDeviceState(isOn);

  // Broadcast to WebSocket clients
  broadcastStatus(isOn);

  // Update Adafruit IO feed if enabled
  if (config.useAdafruitIO && relayFeed) {
    relayFeed->save(isOn ? 1 : 0);
  }
}

// Function to save relay state to EEPROM
void saveDeviceState(bool isOn) {
  EEPROM.begin(4); // Initialize EEPROM with 4 bytes
  uint8_t state = isOn ? 1 : 0;
  EEPROM.write(CONFIG_ADDRESS + sizeof(DeviceConfig), state);
  EEPROM.commit();
}

// Function to broadcast status to all WebSocket clients
void broadcastStatus(bool isOn) {
  String status = isOn ? "ON" : "OFF";
  webSocket.broadcastTXT(status);
}

// Callback function for Adafruit IO feed if configured
void handleRelayFeed(AdafruitIO_Data *data) {
  if (!data) {
    Serial.println("Received null data from Adafruit IO");
    return;
  }

  int relayValue = data->toInt();
  bool newState = (relayValue == 1);

  Serial.print("Received relay state from Adafruit IO: ");
  Serial.println(newState ? "ON" : "OFF");

  if (newState != deviceState) {
    setDeviceState(newState);
  }
}

void setup() {
  // Initialize serial and pins
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  // Load configuration
  loadConfig();

  // WiFi Configuration
  WiFi.persistent(true);
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);

  // Try to connect to WiFi
  bool wifiConnected = attemptWiFiConnection();

  // If WiFi is not connected, start captive portal
  if (!wifiConnected) {
    startCaptivePortal();
    return;
  }

  // Setup mDNS
  setupMDNS();

  // Setup Adafruit IO if configured
  if (config.useAdafruitIO) {
    setupAdafruitIO();
  }

  // Web Server Routes
  server.on("/", []() {
    server.send_P(200, "text/html", WEB_UI);  // Serve main UI from UI.h
  });

  // WebSocket setup
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  server.begin();

  // Set initial device state
  setDeviceState(false);

  Serial.println("Device initialized.");
}

void loop() {
  // DNS server for captive portal
  if (isConfigMode) {
    dnsServer.processNextRequest();
  }

  // Handle web server requests
  server.handleClient();

  // Check WiFi connection if not in config mode
  if (!isConfigMode) {
    checkWiFiConnection();

    // Handle WebSocket
    webSocket.loop();

    // Handle Adafruit IO if enabled
    if (config.useAdafruitIO) {
      io->run();

      // Periodically send IP address
      if (millis() - lastIPUpdateTime > IP_UPDATE_INTERVAL) {
        sendIPToAdafruitIO();
        lastIPUpdateTime = millis();
      }
    }

    // mDNS update
    MDNS.update();
  }
}
