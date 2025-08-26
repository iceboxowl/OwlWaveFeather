#pragma once
#include <Arduino.h>
#include "LoRaWan_APP.h"

// Default RF frequency (override via build_flags -DOW_LORA_RF_FREQUENCY=xxx)
#ifndef OW_LORA_RF_FREQUENCY
#define OW_LORA_RF_FREQUENCY 915000000UL
#endif

class OwLora {
   public:
    void begin();
    bool send(const char* msg);
    bool send(const String& s) {
        return send(s.c_str());
    }

    bool idle() const {
        return m_idle;
    }
    bool initialized() const {
        return m_initialized;
    }

   private:
    static void onTxDone();
    static void onTxTimeout();
    void configureRadio();

    static OwLora* s_instance;

    bool m_initialized = false;
    volatile bool m_idle = true;
    RadioEvents_t m_events;
};