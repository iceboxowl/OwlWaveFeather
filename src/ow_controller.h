#ifndef OWLWAVE_CONTROLLER_H
#define OWLWAVE_CONTROLLER_H

#include <Arduino.h>
#include <RH_RF69.h>

#include "ow_global.h"

class OwlWaveController
{    
public:
    void setupRadio();
    void onRainTip();
    void calculateRainRate();
    void blink(byte PIN, byte DELAY_MS, byte loops);
    void sendBMEMessage(float temperature, float humidity, uint32_t pressure, uint32_t gas);
    void sendRainMessage(); 
    
private:
    // -*-*-*-*-*-*-*-*-*-*-*-*-*-* //
    //      Private Functions       //
    // -*-*-*-*-*-*-*-*-*-*-*-*-*-* //
    void sendMessage(String message);

    // -*-*-*-*-*-*-*-*-*-*-*-*-*-* //
    //      Private Members         //
    // -*-*-*-*-*-*-*-*-*-*-*-*-*-* //
    RH_RF69 *m_rf69;          //!< RFM69 Radio Class.
    uint32_t m_rainTips;      //!< How many times our rain tipper has tipped.
    bool m_rateDecay;         //!< Should our rain rate decay.
    float m_rainRate;         //!< Rain volume reported as in/hr.
    uint32_t m_rainTipMilli;  //!< The last time we got a rain tip (Milliseconds).
                              //!< This value resets if no tip is recieved in 1min.    
    uint32_t m_lastRainTipMilli;                             
};

#endif // OWLWAVE_CONTROLLER_H