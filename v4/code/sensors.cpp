// sensors.cpp
#include "sensors.h"
#include <ArduinoJson.h>
#include <WebSocketsServer.h>

extern WebSocketsServer webSocket;
DHT* dhtSensors[MAX_SENSORS] = {nullptr};
SensorData sensorData[MAX_SENSORS];

void initializeSensors() {
    for (int i = 0; i < config.sensorCount; i++) {
        SensorConfig& sensor = config.sensors[i];
        
        switch (sensor.type) {
            case SENSOR_DHT:
                if (dhtSensors[i] != nullptr) {
                    delete dhtSensors[i];
                }
                dhtSensors[i] = new DHT(sensor.pin, sensor.dhtType);
                dhtSensors[i]->begin();
                break;
                
            case SENSOR_LDR:
            case SENSOR_SOIL:
                pinMode(sensor.pin, INPUT);
                break;
                
            case SENSOR_NONE:
            default:
                break;
        }
    }
}

void readSensors() {
    for (int i = 0; i < config.sensorCount; i++) {
        SensorConfig& sensor = config.sensors[i];
        
        switch (sensor.type) {
            case SENSOR_DHT:
                if (dhtSensors[i] != nullptr) {
                    float temp = dhtSensors[i]->readTemperature();
                    float humidity = dhtSensors[i]->readHumidity();
                    
                    if (!isnan(temp)) {
                        sensorData[i].temperature = temp;
                    }
                    if (!isnan(humidity)) {
                        sensorData[i].humidity = humidity;
                    }
                }
                break;
                
            case SENSOR_LDR:
                {
                    int rawValue = analogRead(sensor.pin);
                    sensorData[i].light = map(rawValue, 0, 1023, 0, 100);
                }
                break;
                
            case SENSOR_SOIL:
                {
                    int rawValue = analogRead(sensor.pin);
                    sensorData[i].moisture = map(rawValue, 0, 1023, 0, 100);
                }
                break;
                
            case SENSOR_NONE:
            default:
                break;
        }
        
        broadcastSensorData(i);
    }
}

void broadcastSensorData(int sensorIndex) {
    if (sensorIndex >= config.sensorCount) return;
    
    StaticJsonDocument<200> doc;
    SensorConfig& sensor = config.sensors[sensorIndex];
    
    doc["sensor"] = sensorIndex;
    doc["type"] = static_cast<int>(sensor.type);
    
    switch (sensor.type) {
        case SENSOR_DHT:
            doc["temperature"] = sensorData[sensorIndex].temperature;
            doc["humidity"] = sensorData[sensorIndex].humidity;
            break;
            
        case SENSOR_LDR:
            doc["light"] = sensorData[sensorIndex].light;
            break;
            
        case SENSOR_SOIL:
            doc["moisture"] = sensorData[sensorIndex].moisture;
            break;
            
        case SENSOR_NONE:
        default:
            return;
    }
    
    String output;
    serializeJson(doc, output);
    webSocket.broadcastTXT(output);
}
