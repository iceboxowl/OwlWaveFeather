#include "ow_controller.h"

void OwlWaveController::blink(byte PIN, byte DELAY_MS, byte loops) 
{
    for (byte i=0; i<loops; i++)  
    {
        digitalWrite(PIN,HIGH);
        delay(DELAY_MS);
        digitalWrite(PIN,LOW);
        delay(DELAY_MS);
    }
}