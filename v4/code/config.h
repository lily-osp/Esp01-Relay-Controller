#ifndef CONFIG_H
#define CONFIG_H

#include <AdafruitIO_WiFi.h>
#include <DHT.h>
#include <DNSServer.h>
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WebSocketsServer.h>
#include "led.h"

// Device limits and intervals
#define MAX_RELAYS 4
#define MAX_SENSORS 6
#define SENSOR_UPDATE_INTERVAL 5000
#define IP_UPDATE_INTERVAL (5 * 60 * 1000)

// WiFi and Network Constants
#define WIFI_CONNECT_TIMEOUT 15000
#define WIFI_RETRY_INTERVAL 30000
#define MAX_WIFI_CONNECT_ATTEMPTS 5
#define DNS_PORT 53

// Device Constants
#define RELAY_PIN 0
#define LED_PIN 1
#define CONFIG_ADDRESS 0
#define AP_SSID "ESP8266-Setup"
#define AP_PASSWORD "configme123"

// Forward declarations
class AdafruitIO_Feed;
void setDeviceState(bool state);
void loadDeviceState();
void initializeSensors();
void readSensors();
void setupMDNS();
void setupAdafruitIO();
void startCaptivePortal();
bool attemptWiFiConnection();
void checkWiFiConnection();
void sendIPToAdafruitIO();
void broadcastStatus(bool state);
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);

// Sensor Types
enum SensorType {
    SENSOR_NONE = 0,
    SENSOR_DHT = 1,
    SENSOR_LDR = 2,
    SENSOR_SOIL = 3
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

// External variables
extern DeviceConfig config;
extern ESP8266WebServer server;
extern DNSServer dnsServer;
extern WebSocketsServer webSocket;
extern AdafruitIO_WiFi* io;
extern AdafruitIO_Feed* relayFeed;
extern AdafruitIO_Feed* ipFeed;
extern bool deviceState;
extern bool isSetupMode;
extern unsigned long lastSensorUpdate;
extern unsigned long lastIPUpdate;

void saveConfig();
void loadConfig();

#endif