#include "ow_controller.h"
#include "ow_message_types.h"

char output[256];

void OwlWaveController::calculateRainRate()
{
    // First check if our millis() has rolled over (Happens once every 49 days)
    // Reset our m_lastRainTipMilli if this happens so our comparison doesn't come out negative.
    if (m_rainTipMilli < m_lastRainTipMilli)
    {
        m_lastRainTipMilli = 0;
    }
    else if (m_rainTipMilli == m_lastRainTipMilli)
    {
        m_rainRate = 0;
        return;
    }

    // Rain rate will always be 0 if its been 5min since we last saw a tip.
    if ((millis() - m_rainTipMilli) > 3000000)
    {
        m_rainRate = 0;
        return;
    }

    // We need to calculate rain rate differently if we got a tip
    // right before the rain rate gets calculated. This will also allow us to display rain rate decay.
    if (m_rateDecay)
    {
        uint32_t rainDiff = millis() - m_lastRainTipMilli;
        m_rainRate = (3600000 / rainDiff) * RAIN_TIP_AMOUNT;
    }
    else
    {
        uint32_t rainDiff = m_rainTipMilli - m_lastRainTipMilli;
        m_rainRate = (3600000 / rainDiff) * RAIN_TIP_AMOUNT;
    }
    
    m_rateDecay = true; // Make sure we enable rate decay at the end of each calculation.
}

void OwlWaveController::onRainTip()
{
    m_lastRainTipMilli = m_rainTipMilli;
    m_rainTips++;
    m_rainTipMilli = millis();
    m_rateDecay = false;
}

void OwlWaveController::setupRadio()
{
    if (!m_rf69) m_rf69 = new RH_RF69(RFM69_CS, RFM69_INT);

    if (!m_rf69->init()) 
    {
        Serial.println("RFM69 radio init failed");
        return; // don't hang; main code can detect lack of radio
    }
    m_radioInitialized = true;
    Serial.println("RFM69 radio init OK!");
    if (!m_rf69->setFrequency(RF69_FREQ)) 
    {
        Serial.println("setFrequency failed");
    }
    m_rf69->setTxPower(m_txPower, true);  // 14-20 for HCW
    uint8_t key[] = { 0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,
                      0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08 };
    m_rf69->setEncryptionKey(key);
    Serial.print("RFM69 radio @");  Serial.print((int)RF69_FREQ);  Serial.println(" MHz");
}

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

void OwlWaveController::sendBMEMessage(float temperature, float humidity, uint32_t pressure, uint32_t gas)
{
    // Message Types
    String messageTypeTemperature = String(OwlWaveMessageTypes::TEMPERATURE) + "=" + String(temperature, 2) + String(OW_MESSAGE_SEPERATOR);
    String messageTypeHumidity = String(OwlWaveMessageTypes::HUMIDITY) + "=" + String(humidity, 2) + String(OW_MESSAGE_SEPERATOR);
    String messageTypePressure = String(OwlWaveMessageTypes::PRESSURE) + "=" + String((pressure / 100.0), 2) + String(OW_MESSAGE_SEPERATOR);
    String messageTypeGas = String(OwlWaveMessageTypes::GAS) + "=" + String((gas / 1000.0), 2) + String(" ");

    sendMessage(messageTypeTemperature + messageTypeHumidity + messageTypePressure + messageTypeGas);
}

void OwlWaveController::sendRainMessage()
{
    String messageTypeRainTips = String(OwlWaveMessageTypes::RAINTIPS) + "=" + String(m_rainTips) + String(OW_MESSAGE_SEPERATOR);
    String messageTypeRainRate = String(OwlWaveMessageTypes::RAINRATE) + "=" + String(m_rainRate, 4) + String(" ");

    sendMessage(messageTypeRainTips + messageTypeRainRate);
}

void OwlWaveController::sendMessage(String message)
{
    uint8_t messageLength = message.length();

    char buffer[messageLength + 1] = ""; // init our char array with an empty string.

    message.toCharArray(buffer, messageLength);

    Serial.print("Sending "); Serial.println(buffer); // Debug message;

    if (m_radioInitialized && m_rf69)
    {
        m_rf69->send((uint8_t *)buffer, strlen(buffer));
        m_rf69->waitPacketSent();
    }
}

void OwlWaveController::sleepRadio()
{
    if (m_radioInitialized && m_rf69)
    {
        m_rf69->sleep(); // true low-power sleep (couple µA)
    }
}

void OwlWaveController::wakeRadio()
{
    if (m_radioInitialized && m_rf69)
    {
        // Standby (modeIdle) wakes oscillator quickly
        m_rf69->setModeIdle();
        // Reapply critical params (defensive)
        m_rf69->setFrequency(RF69_FREQ);
        m_rf69->setTxPower(m_txPower, true);
    }
}