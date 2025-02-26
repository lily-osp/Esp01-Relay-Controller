#include "webserver.h"
#include "UI.h"
#include "led.h"
#include <ArduinoJson.h>

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
    // Save WiFi configuration
    strncpy(config.wifiSSID, server.arg("wifiSSID").c_str(), sizeof(config.wifiSSID) - 1);
    strncpy(config.wifiPassword, server.arg("wifiPassword").c_str(), sizeof(config.wifiPassword) - 1);
    strncpy(config.mdnsName, server.arg("mdnsName").c_str(), sizeof(config.mdnsName) - 1);

    // Save Adafruit IO configuration
    config.useAdafruitIO = (server.arg("useAdafruitIO") == "true");
    if (config.useAdafruitIO) {
        strncpy(config.ioUsername, server.arg("ioUsername").c_str(), sizeof(config.ioUsername) - 1);
        strncpy(config.ioKey, server.arg("ioKey").c_str(), sizeof(config.ioKey) - 1);
        strncpy(config.relayFeedName, server.arg("relayFeedName").c_str(), sizeof(config.relayFeedName) - 1);
        strncpy(config.ipFeedName, server.arg("ipFeedName").c_str(), sizeof(config.ipFeedName) - 1);
    }

    // Save relay configuration
    config.relayCount = server.arg("relayCount").toInt();
    for (int i = 0; i < config.relayCount; i++) {
        config.relayPins[i] = server.arg("relayPin" + String(i)).toInt();
    }

    // Save sensor configuration
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

    // Save configuration to EEPROM
    saveConfig();
    startLedPattern(LED_PATTERN_RESET);

    // Notify user and reboot
    server.send(200, "text/plain", "Configuration saved! Rebooting...");
    delay(1000);
    ESP.restart();
}

void initWebServer() {
    // Set up web server routes
    server.on("/", []() {
        server.send_P(200, "text/html", WEB_UI);
    });
    
    server.onNotFound(handleNotFound);
    
    // Start WebSocket server
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    
    // Start web server
    server.begin();
    
    Serial.println("WebSocket server started on port 81");
    Serial.println("Web server started");
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case WStype_DISCONNECTED:
            Serial.printf("[%u] Disconnected!\n", num);
            break;
            
        case WStype_CONNECTED:
            {
                Serial.printf("[%u] Connected from url: %s\n", num, payload);
                
                // Send current relay states
                for (int i = 0; i < config.relayCount; i++) {
                    StaticJsonDocument<200> doc;
                    doc["type"] = "relay";
                    doc["index"] = i;
                    doc["state"] = digitalRead(config.relayPins[i]) == LOW;
                    
                    String jsonString;
                    serializeJson(doc, jsonString);
                    webSocket.sendTXT(num, jsonString);
                }
            }
            break;
            
        case WStype_TEXT:
            {
                StaticJsonDocument<512> doc;
                DeserializationError error = deserializeJson(doc, payload);
                
                if (error) {
                    Serial.print(F("deserializeJson() failed: "));
                    Serial.println(error.f_str());
                    return;
                }
                
                const char* msgType = doc["type"];
                
                if (strcmp(msgType, "relay") == 0) {
                    int index = doc["index"];
                    bool state = doc["state"];
                    
                    if (index >= 0 && index < config.relayCount) {
                        digitalWrite(config.relayPins[index], !state); // Invert state because relays are active LOW
                        
                        // Broadcast the new state to all clients
                        StaticJsonDocument<200> response;
                        response["type"] = "relay";
                        response["index"] = index;
                        response["state"] = state;
                        
                        String jsonString;
                        serializeJson(response, jsonString);
                        webSocket.broadcastTXT(jsonString);
                    }
                }
            }
            break;
    }
}