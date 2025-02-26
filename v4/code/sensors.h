// sensors.h
#ifndef SENSORS_H
#define SENSORS_H

#include "config.h"
#include <DHT.h>
#include <DHT_U.h>
#include <Adafruit_Sensor.h>

// DHT sensor types
enum DHTType {
    DHT_11 = 11,
    DHT_22 = 22,
    DHT_21 = 21
};

// Sensor data structure
struct SensorData {
    float temperature;
    float humidity;
    float moisture;
    float light;
    float analog;
};

extern SensorData sensorData[MAX_SENSORS];

void initializeSensors();
void readSensors();
void broadcastSensorData(int sensorIndex);

#endif
