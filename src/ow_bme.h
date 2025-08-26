#pragma once
#include <Arduino.h>
#include <Wire.h>
#include "BME680.h"  // BME680_Class

struct OwBmeReading {
    float temperatureC;
    float humidityPct;
    float pressureHpa;
    float gasKOhms;
    uint32_t timestampMs;
};

class OwBme {
   public:
    bool begin();
    OwBmeReading readAll();

   private:
    bool m_ready = false;
    BME680_Class m_bme;
};