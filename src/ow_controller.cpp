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
    m_rf69 = new RH_RF69(RFM69_CS, RFM69_INT);

    if (!m_rf69->init()) 
    {
      Serial.println("RFM69 radio init failed");
      while (1);
    }
    Serial.println("RFM69 radio init OK!");
    // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM (for low power module)
    // No encryption
    if (!m_rf69->setFrequency(RF69_FREQ)) 
    {
      Serial.println("setFrequency failed");
    }

    // If you are using a high power RF69 eg RFM69HW, you *must* set a Tx power with the
    // ishighpowermodule flag set like this:
    m_rf69->setTxPower(20, true);  // range from 14-20 for power, 2nd arg must be true for 69HCW

    // The encryption key has to be the same as the one in the server
    uint8_t key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                      0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    m_rf69->setEncryptionKey(key);

    Serial.print("RFM69 radio @");  Serial.print((int)RF69_FREQ);  Serial.println(" MHz");
}

void OwlWaveController::setupAQSensor()
{
    // Initialize the sensor with the serial device
    // that it's connected to. Hardware Serial1 is the
    // default, if no parameter is provided to init()

    Serial1.begin(9600);    // Note: 9600 baud
    m_sensor.init(&Serial1);
    //m_sensor.debug = true;
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

void OwlWaveController::updateAQ()
{
    m_sensor.updateFrame();
}

void OwlWaveController::sendAQMessage()
{
    // Use updateFrame() to read in sensor data in your
    // loop until hasNewData() returns true.
    m_sensor.updateFrame();

    // Note: once updateFrame() is called, all data is
    // invalid until hasNewData() returns true.
    if (m_sensor.hasNewData())
    {
        uint16_t pm1 = m_sensor.getPM_1_0();
        uint16_t pm2 = m_sensor.getPM_2_5();
        uint16_t pm10 = m_sensor.getPM_10_0();

        String messageTypePM1 = String(OwlWaveMessageTypes::AIRQUALITY_PM1) + "=" + String(pm1) + String(OW_MESSAGE_SEPERATOR);
        String messageTypePM2_5 = String(OwlWaveMessageTypes::AIRQUALITY_PM2_5) + "=" + String(pm2) + String(OW_MESSAGE_SEPERATOR);
        String messageTypePM10 = String(OwlWaveMessageTypes::AIRQUALITY_PM10) + "=" + String(pm10) + String(" ");

        sendMessage(messageTypePM1 + messageTypePM2_5 + messageTypePM10);
    }
}

void OwlWaveController::debugAQ()
{
        // Use updateFrame() to read in sensor data in your
    // loop until hasNewData() returns true.
    m_sensor.updateFrame();

    // Note: once updateFrame() is called, all data is
    // invalid until hasNewData() returns true.
    if (m_sensor.hasNewData())
    {
        sprintf(output, "\nSensor Version: %d    Error Code: %d\n",
                  m_sensor.getHWVersion(),
                  m_sensor.getErrorCode());
        Serial.print(output);

        sprintf(output, "    PM1.0 (ug/m3): %2d     [atmos: %d]\n",
                    m_sensor.getPM_1_0(),
                    m_sensor.getPM_1_0_atmos());              
        Serial.print(output);
        sprintf(output, "    PM2.5 (ug/m3): %2d     [atmos: %d]\n",
                    m_sensor.getPM_2_5(),
                    m_sensor.getPM_2_5_atmos());
        Serial.print(output);
        sprintf(output, "    PM10  (ug/m3): %2d     [atmos: %d]\n",
                    m_sensor.getPM_10_0(),
                    m_sensor.getPM_10_0_atmos());              
        Serial.print(output);

        sprintf(output, "\n    RAW: %2d[>0.3] %2d[>0.5] %2d[>1.0] %2d[>2.5] %2d[>5.0] %2d[>10]\n",
                    m_sensor.getRawGreaterThan_0_3(),
                    m_sensor.getRawGreaterThan_0_5(),
                    m_sensor.getRawGreaterThan_1_0(),
                    m_sensor.getRawGreaterThan_2_5(),
                    m_sensor.getRawGreaterThan_5_0(),
                    m_sensor.getRawGreaterThan_10_0());
        Serial.print(output);
    }
}

void OwlWaveController::sendMessage(String message)
{
    uint8_t messageLength = message.length();

    char buffer[messageLength + 1] = ""; // init our char array with an empty string.

    message.toCharArray(buffer, messageLength);

    Serial.print("Sending "); Serial.println(buffer); // Debug message;

    m_rf69->send((uint8_t *)buffer, strlen(buffer));
    m_rf69->waitPacketSent();
    blink(LED, 50, 1);
}