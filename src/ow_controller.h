// #ifndef OWLWAVE_CONTROLLER_H
// #define OWLWAVE_CONTROLLER_H

// #include <Arduino.h>
// #include <RH_RF69.h>

// #include "ow_global.h"

// class OwlWaveController
// {    
// public:
//     void setupRadio();
//     //void setupAQSensor();
//     // Use a caller-supplied monotonic system time (ms). millis() may not advance during deep sleep,
//     // so callers (main.cpp) should pass the watchdog-accumulated time reference.
//     void onRainTip(uint32_t currentTimeMs);
//     // Handle multiple rain tips that occurred while sleeping. The implementation will distribute
//     // timestamps between the last known tip and currentSystemMs to preserve a reasonable rate.
//     void onRainTips(uint32_t count, uint32_t currentSystemMs);
//     void calculateRainRate(uint32_t currentTimeMs);
//     void blink(byte PIN, byte DELAY_MS, byte loops);
//     void sendBMEMessage(float temperature, float humidity, uint32_t pressure, uint32_t gas);
//     void sendRainMessage(); 
//     // Send BME sensor data and rain data together in a single packet
//     void sendAllData(float temperature, float humidity, uint32_t pressure, uint32_t gas, uint32_t currentSystemMs);
//     void sleepRadio();
//     void wakeRadio();
    
// private:
//     // -*-*-*-*-*-*-*-*-*-*-*-*-*-* //
//     //      Private Functions       //
//     // -*-*-*-*-*-*-*-*-*-*-*-*-*-* //
//     void sendMessage(String message);

//     // -*-*-*-*-*-*-*-*-*-*-*-*-*-* //
//     //      Private Members         //
//     // -*-*-*-*-*-*-*-*-*-*-*-*-*-* //
//     RH_RF69 *m_rf69 = nullptr;       //!< RFM69 Radio Class.
//     bool m_radioInitialized = false; //!< Tracks successful radio init.
//     uint8_t m_txPower = 20;          //!< Cached TX power setting.
//     volatile uint32_t m_rainTips = 0;    //!< Updated inside interrupt.
//     volatile bool m_rateDecay = false;   //!< Updated inside interrupt.
//     float m_rainRate = 0.0f;             //!< Rain volume reported as in/hr.
//     volatile uint32_t m_rainTipMilli = 0;//!< Last rain tip time (ms relative to system time).
//     volatile uint32_t m_lastRainTipMilli = 0;
// };

// #endif // OWLWAVE_CONTROLLER_H