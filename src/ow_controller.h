#ifndef OWLWAVE_CONTROLLER_H
#define OWLWAVE_CONTROLLER_H

#include <Arduino.h>
#include <RH_RF69.h>

#include "ow_global.h"

class OwlWaveController
{    
public:
    void setupRadio();
    //void setupAQSensor();
    void onRainTip();
    void calculateRainRate();
    void blink(byte PIN, byte DELAY_MS, byte loops);
    void sendBMEMessage(float temperature, float humidity, uint32_t pressure, uint32_t gas);
    void sendRainMessage(); 
    void sleepRadio();
    void wakeRadio();
    
private:
    // -*-*-*-*-*-*-*-*-*-*-*-*-*-* //
    //      Private Functions       //
    // -*-*-*-*-*-*-*-*-*-*-*-*-*-* //
    void sendMessage(String message);

    // -*-*-*-*-*-*-*-*-*-*-*-*-*-* //
    //      Private Members         //
    // -*-*-*-*-*-*-*-*-*-*-*-*-*-* //
    RH_RF69 *m_rf69 = nullptr;       //!< RFM69 Radio Class.
    bool m_radioInitialized = false; //!< Tracks successful radio init.
    uint8_t m_txPower = 20;          //!< Cached TX power setting.
    volatile uint32_t m_rainTips;    //!< Updated inside interrupt.
    volatile bool m_rateDecay;       //!< Updated inside interrupt.
    float m_rainRate;                //!< Rain volume reported as in/hr.
    volatile uint32_t m_rainTipMilli;//!< Last rain tip time (millis()).
    volatile uint32_t m_lastRainTipMilli;
};

#endif // OWLWAVE_CONTROLLER_H