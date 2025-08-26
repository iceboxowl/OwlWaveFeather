#include "ow_lora.h"

// LoRa modulation parameters (adjust as needed)
#define OW_LORA_TX_POWER 14
#define OW_LORA_BANDWIDTH 0  // 125 kHz
#define OW_LORA_SF 7
#define OW_LORA_CR 1  // 4/5
#define OW_LORA_PREAMBLE_LEN 8
#define OW_LORA_FIX_LEN_PAYLOAD false
#define OW_LORA_IQ_INVERT false
#define OW_LORA_TIMEOUT_MS 3000
#define OW_LORA_BUFFER_SIZE 128

OwLora* OwLora::s_instance = nullptr;

void OwLora::begin() {
    if (m_initialized) return;
    s_instance = this;

    m_events.TxDone = &OwLora::onTxDone;
    m_events.TxTimeout = &OwLora::onTxTimeout;

    Radio.Init(&m_events);
    configureRadio();

    m_idle = true;
    m_initialized = true;
#if DEBUG_SERIAL
    Serial.println("LoRa init OK");
#endif
}

void OwLora::configureRadio() {
    Radio.SetChannel(OW_LORA_RF_FREQUENCY);
    Radio.SetTxConfig(MODEM_LORA, OW_LORA_TX_POWER, 0, OW_LORA_BANDWIDTH,
                      OW_LORA_SF, OW_LORA_CR, OW_LORA_PREAMBLE_LEN,
                      OW_LORA_FIX_LEN_PAYLOAD,
                      true,  // CRC
                      0, 0, OW_LORA_IQ_INVERT, OW_LORA_TIMEOUT_MS);
}

bool OwLora::send(const char* msg) {
    if (!m_initialized) return false;
    if (!m_idle) return false;
    if (!msg) return false;
    size_t len = strlen(msg);
    if (len == 0 || len >= OW_LORA_BUFFER_SIZE) return false;

#if DEBUG_SERIAL
    Serial.print("LoRa TX: ");
    Serial.println(msg);
#endif
    m_idle = false;
    Radio.Send((uint8_t*)msg, len);
    return true;
}

void OwLora::onTxDone() {
    if (s_instance) {
        s_instance->m_idle = true;
#if DEBUG_SERIAL
        Serial.println("LoRa TX done");
#endif
    }
}

void OwLora::onTxTimeout() {
    Radio.Sleep();
    if (s_instance) {
        s_instance->m_idle = true;
#if DEBUG_SERIAL
        Serial.println("LoRa TX timeout");
#endif
    }
}