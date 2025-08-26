// #include "ow_controller.h"
// #include "ow_message_types.h"

// char output[256];

// // void OwlWaveController::calculateRainRate()
// // {
// //     // This function is now deprecated without a time parameter. Keep old symbol to avoid link errors.
// //     // Prefer calling calculateRainRate(currentTimeMs).
// // }

// void OwlWaveController::calculateRainRate(uint32_t currentTimeMs)
// {
//     // First check rollover relative to last tip timestamp
//     if (m_rainTipMilli < m_lastRainTipMilli)
//     {
//         m_lastRainTipMilli = 0;
//     }
//     else if (m_rainTipMilli == m_lastRainTipMilli)
//     {
//         m_rainRate = 0;
//         return;
//     }

//     // If it's been more than 5 minutes (300000 ms) since last tip, rate = 0
//     if ((currentTimeMs - m_rainTipMilli) > 300000UL)
//     {
//         m_rainRate = 0;
//         return;
//     }

//     // Calculate rain rate. If rate decay is active we measure from last active time to now.
//     uint32_t rainDiff = 0;
//     if (m_rateDecay)
//     {
//         // use currentTimeMs relative to last recorded tip
//         rainDiff = currentTimeMs - m_lastRainTipMilli;
//     }
//     else
//     {
//         rainDiff = m_rainTipMilli - m_lastRainTipMilli;
//     }

//     if (rainDiff == 0)
//     {
//         m_rainRate = 0;
//     }
//     else
//     {
//         m_rainRate = (3600000.0f / (float)rainDiff) * RAIN_TIP_AMOUNT;
//     }

//     m_rateDecay = true; // enable decay after calculation
// }

// void OwlWaveController::onRainTip(uint32_t currentTimeMs)
// {
//     m_lastRainTipMilli = m_rainTipMilli;
//     m_rainTips++;
//     m_rainTipMilli = currentTimeMs;
//     m_rateDecay = false;
// }

// void OwlWaveController::onRainTips(uint32_t count, uint32_t currentSystemMs)
// {
//     if (count == 0) return;

//     // If we have no prior tip timestamp, just stamp them at currentSystemMs
//     if (m_rainTipMilli == 0 && m_lastRainTipMilli == 0)
//     {
//         // All tips happened since boot; treat lastTip as currentSystemMs - small delta
//         for (uint32_t i = 0; i < count; ++i)
//         {
//             m_lastRainTipMilli = m_rainTipMilli;
//             m_rainTips++;
//             m_rainTipMilli = currentSystemMs; // all same time
//             m_rateDecay = false;
//         }
//         return;
//     }

//     // Distribute the timestamps over the interval from m_rainTipMilli (last recorded tip) to currentSystemMs
//     // This gives a reasonable approximation for rain rate when multiple tips occurred while asleep.
//     uint32_t start = m_rainTipMilli;
//     uint32_t end = currentSystemMs;
//     if (end <= start) end = start + 1; // avoid division by zero

//     // Interval between distributed tips
//     float step = (float)(end - start) / (float)count;

//     for (uint32_t i = 0; i < count; ++i)
//     {
//         m_lastRainTipMilli = m_rainTipMilli;
//         m_rainTips++;
//         // New tip time is start + (i+1)*step
//         uint32_t newTip = start + (uint32_t)((i + 1) * step);
//         m_rainTipMilli = newTip;
//         m_rateDecay = false;
//     }
// }

// void OwlWaveController::setupRadio()
// {
//     if (!m_rf69) m_rf69 = new RH_RF69(RFM69_CS, RFM69_INT);

//     if (!m_rf69->init()) 
//     {
//         Serial.println("RFM69 radio init failed");
//         return; // don't hang; main code can detect lack of radio
//     }
//     m_radioInitialized = true;
//     Serial.println("RFM69 radio init OK!");
//     if (!m_rf69->setFrequency(RF69_FREQ)) 
//     {
//         Serial.println("setFrequency failed");
//     }
//     m_rf69->setTxPower(m_txPower, true);  // 14-20 for HCW
//     uint8_t key[] = { 0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,
//                       0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08 };
//     m_rf69->setEncryptionKey(key);
//     Serial.print("RFM69 radio @");  Serial.print((int)RF69_FREQ);  Serial.println(" MHz");
// }

// void OwlWaveController::blink(byte PIN, byte DELAY_MS, byte loops) 
// {
//     for (byte i=0; i<loops; i++)  
//     {
//         digitalWrite(PIN,HIGH);
//         delay(DELAY_MS);
//         digitalWrite(PIN,LOW);
//         delay(DELAY_MS);
//     }
// }

// void OwlWaveController::sendBMEMessage(float temperature, float humidity, uint32_t pressure, uint32_t gas)
// {
//     // Message Types
//     String messageTypeTemperature = String(OwlWaveMessageTypes::TEMPERATURE) + "=" + String(temperature, 2) + String(OW_MESSAGE_SEPERATOR);
//     String messageTypeHumidity = String(OwlWaveMessageTypes::HUMIDITY) + "=" + String(humidity, 2) + String(OW_MESSAGE_SEPERATOR);
//     String messageTypePressure = String(OwlWaveMessageTypes::PRESSURE) + "=" + String((pressure / 100.0), 2) + String(OW_MESSAGE_SEPERATOR);
//     String messageTypeGas = String(OwlWaveMessageTypes::GAS) + "=" + String((gas / 1000.0), 2) + String(" ");

//     sendMessage(messageTypeTemperature + messageTypeHumidity + messageTypePressure + messageTypeGas);
// }

// void OwlWaveController::sendRainMessage()
// {
//     String messageTypeRainTips = String(OwlWaveMessageTypes::RAINTIPS) + "=" + String(m_rainTips) + String(OW_MESSAGE_SEPERATOR);
//     String messageTypeRainRate = String(OwlWaveMessageTypes::RAINRATE) + "=" + String(m_rainRate, 4) + String(" ");

//     sendMessage(messageTypeRainTips + messageTypeRainRate);
// }

// void OwlWaveController::sendAllData(float temperature, float humidity, uint32_t pressure, uint32_t gas, uint32_t currentSystemMs)
// {
//     // Ensure rain rate is calculated against the provided time reference before composing message
//     calculateRainRate(currentSystemMs);

//     String messageTypeTemperature = String(OwlWaveMessageTypes::TEMPERATURE) + "=" + String(temperature, 2) + String(OW_MESSAGE_SEPERATOR);
//     String messageTypeHumidity = String(OwlWaveMessageTypes::HUMIDITY) + "=" + String(humidity, 2) + String(OW_MESSAGE_SEPERATOR);
//     String messageTypePressure = String(OwlWaveMessageTypes::PRESSURE) + "=" + String((pressure / 100.0), 2) + String(OW_MESSAGE_SEPERATOR);
//     String messageTypeGas = String(OwlWaveMessageTypes::GAS) + "=" + String((gas / 1000.0), 2) + String(OW_MESSAGE_SEPERATOR);
//     String messageTypeRainTips = String(OwlWaveMessageTypes::RAINTIPS) + "=" + String(m_rainTips) + String(OW_MESSAGE_SEPERATOR);
//     String messageTypeRainRate = String(OwlWaveMessageTypes::RAINRATE) + "=" + String(m_rainRate, 4) + String(" ");

//     String full = messageTypeTemperature + messageTypeHumidity + messageTypePressure + messageTypeGas + messageTypeRainTips + messageTypeRainRate;
//     sendMessage(full);
// }

// void OwlWaveController::sendMessage(String message)
// {
//     uint16_t messageLength = message.length();

//     // Create buffer large enough for the full message plus null terminator.
//     char buffer[messageLength + 1];
//     // toCharArray expects the buffer size (including null terminator).
//     message.toCharArray(buffer, messageLength + 1);

//     Serial.print("Sending "); Serial.println(buffer); // Debug message;

//     if (m_radioInitialized && m_rf69)
//     {
//         // Send exact message length (do not rely on strlen which may be affected by embedded nulls).
//         m_rf69->send((uint8_t *)buffer, messageLength);
//         m_rf69->waitPacketSent();
//     }
// }

// void OwlWaveController::sleepRadio()
// {
//     if (m_radioInitialized && m_rf69)
//     {
//         m_rf69->sleep(); // true low-power sleep (couple µA)
//     }
// }

// void OwlWaveController::wakeRadio()
// {
//     if (m_radioInitialized && m_rf69)
//     {
//         // Standby (modeIdle) wakes oscillator quickly
//         m_rf69->setModeIdle();
//         // Reapply critical params (defensive)
//         m_rf69->setFrequency(RF69_FREQ);
//         m_rf69->setTxPower(m_txPower, true);
//     }
// }