#include "ow_controller.h"
#include "ow_message_types.h"

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

String OwlWaveController::getBMEMessage(float temperature, float humidity, uint32_t pressure, uint32_t gas)
{
    // Message Types
    String messageTypeTemperature = String(OwlWaveMessageTypes::TEMPERATURE) + "=" + String(temperature, 2) + String(OW_MESSAGE_SEPERATOR);
    String messageTypeHumidity = String(OwlWaveMessageTypes::HUMIDITY) + "=" + String(humidity, 2) + String(OW_MESSAGE_SEPERATOR);
    String messageTypePressure = String(OwlWaveMessageTypes::PRESSURE) + "=" + String((pressure / 100.0), 2) + String(OW_MESSAGE_SEPERATOR);
    String messageTypeGas = String(OwlWaveMessageTypes::GAS) + "=" + String((gas / 1000.0), 2);

    return messageTypeTemperature + messageTypeHumidity + messageTypePressure + messageTypeGas;
}