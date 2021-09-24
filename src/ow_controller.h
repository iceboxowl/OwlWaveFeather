#ifndef OWLWAVE_CONTROLLER_H
#define OWLWAVE_CONTROLLER_H

#include <Arduino.h>

class OwlWaveController
{
public:
    void blink(byte PIN, byte DELAY_MS, byte loops);
};

#endif // OWLWAVE_CONTROLLER_H