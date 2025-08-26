#include "ow_bme.h"

bool OwBme::begin() {
    Wire.begin();
    for (uint8_t i = 0; i < 5; ++i) {
        if (m_bme.begin(I2C_STANDARD_MODE)) {
            m_ready = true;
            break;
        }
        delay(200);
    }
    if (!m_ready) return false;

    m_bme.setOversampling(TemperatureSensor, Oversample16);
    m_bme.setOversampling(HumiditySensor, Oversample16);
    m_bme.setOversampling(PressureSensor, Oversample16);
    m_bme.setIIRFilter(IIR4);
    m_bme.setGas(320, 150);
    return true;
}

OwBmeReading OwBme::readAll() {
    OwBmeReading r{};
    r.timestampMs = millis();
    if (!m_ready) return r;

    int32_t t, h, p, g;
    m_bme.getSensorData(t, h, p, g);

    r.temperatureC = t / 100.0f;
    r.humidityPct = h / 1000.0f;
    r.pressureHpa = p / 100.0f;
    r.gasKOhms = (g / 100.0f) / 1000.0f;
    return r;
}