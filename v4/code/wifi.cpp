// wifi.cpp
#include "wifi.h"
#include "webserver.h"
#include "led.h"

extern bool isSetupMode;

void startCaptivePortal()
{
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASSWORD);

    dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

    server.on("/", handleSetup);
    server.on("/save-config", HTTP_POST, handleSaveConfig);
    server.on("/enter-setup", handleSetupMode);
    server.onNotFound(handleNotFound);
    
    server.begin();
    
    isSetupMode = true;
    startLedPattern(LED_PATTERN_SETUP);
}

bool attemptWiFiConnection()
{
    if (strlen(config.wifiSSID) == 0) {
        startLedPattern(LED_PATTERN_ERROR);
        return false;
    }

    startLedPattern(LED_PATTERN_WIFI_CONNECTING);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(config.wifiSSID, config.wifiPassword);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < MAX_WIFI_CONNECT_ATTEMPTS) {
        delay(WIFI_CONNECT_TIMEOUT / MAX_WIFI_CONNECT_ATTEMPTS);
        attempts++;
        
        if (attempts == MAX_WIFI_CONNECT_ATTEMPTS) {
            startLedPattern(LED_PATTERN_ERROR);
            return false;
        }
    }
    
    startLedPattern(LED_PATTERN_SUCCESS);
    return true;
}

void checkWiFiConnection()
{
    if (isSetupMode)
        return;
        
    static unsigned long lastWiFiCheck = 0;
    unsigned long currentMillis = millis();
    
    if (currentMillis - lastWiFiCheck >= WIFI_RETRY_INTERVAL) {
        lastWiFiCheck = currentMillis;
        
        if (WiFi.status() != WL_CONNECTED) {
            startLedPattern(LED_PATTERN_ERROR);
            
            WiFi.disconnect();
            delay(1000);
            
            startLedPattern(LED_PATTERN_WIFI_CONNECTING);
            WiFi.begin(config.wifiSSID, config.wifiPassword);
        }
    }
}

void setupMDNS()
{
    if (MDNS.begin(config.mdnsName)) {
        MDNS.addService("http", "tcp", 80);
        startLedPattern(LED_PATTERN_SUCCESS);
    } else {
        startLedPattern(LED_PATTERN_ERROR);
    }
}
