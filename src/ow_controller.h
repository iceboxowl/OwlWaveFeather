#ifndef OWLWAVE_CONTROLLER_H
#define OWLWAVE_CONTROLLER_H

#include <Arduino.h>

class OwlWaveController
{
public:
    void blink(byte PIN, byte DELAY_MS, byte loops);
    String getBMEMessage(float temperature, float humidity, uint32_t pressure, uint32_t gas);
};

#endif // OWLWAVE_CONTROLLER_H