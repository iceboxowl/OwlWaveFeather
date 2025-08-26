#include "Arduino.h"
#include "LoRa_APP.h"
#include "CubeCell_NeoPixel.h"
#include "ow_bme.h"
#include "ow_lora.h"

static TimerEvent_t sleepTimer;
static uint8_t lowPowerMode = 1;
static volatile uint32_t rainTips = 0;
static volatile uint32_t lastRainTipTime = 0;
static float rainRate = 0.0f;
static bool serialReady = true;  // track if Serial was initialized

// OPTIONAL: comment this out (or set to 0) for real low-power
#define DEBUG_SERIAL 1
#define WAKEUP_PIN GPIO0
#define TIMETILL_SLEEP 2400

CubeCell_NeoPixel rgbPixel(1, RGB, NEO_GRB + NEO_KHZ800);
static OwBme bme;
static OwLora lora;
static bool bmeReady = false;

// Function to blink the LED
void blinkLED(uint8_t pin = Vext, uint8_t delayMs = 240, uint8_t loops = 1) {
    rgbPixel.clear();
    delay(delayMs);

    // Dark Purple
    rgbPixel.setPixelColor(0, rgbPixel.Color(30, 0, 60));
    rgbPixel.show();
    delay(delayMs);

    rgbPixel.clear();
    rgbPixel.show();  // Ensure the LED turns off
    delay(delayMs);
}

// Function to calculate rain decay
void calculateRainRate(uint32_t currentTimeMs) {
    if (lastRainTipTime == 0 || (currentTimeMs - lastRainTipTime) > 300000UL) {
        rainRate = 0.0f;
    } else {
        uint32_t timeDiff = currentTimeMs - lastRainTipTime;
        rainRate = (3600000.0f / timeDiff) * rainTips;
    }
}

// Build & send weather payload (fits in <=128 bytes)
static void sendWeatherPacket(const OwBmeReading& r) {
    char buf[120];
    // r.gasKOhms may be 0 if not ready; safe
    snprintf(buf, sizeof(buf),
             "T=%.2fC,H=%.2f%%,P=%.2fhPa,Gas=%.3fk,rTips=%lu,rRate=%.3f/hr",
             r.temperatureC, r.humidityPct, r.pressureHpa, r.gasKOhms,
             (unsigned long)rainTips, rainRate);
    lora.send(buf);
}

// Re-init Serial after wake (CubeCell deep sleep powers down USB/UART)
static void reinitSerial() {
    if (serialReady) return;  // already up
    Serial.begin(115200);
    delay(50);  // allow port to enumerate/stabilize
    serialReady = true;
}

// Helper: put radio & peripherals into lowest state
static void enterPeripheralsSleep() {
#if DEBUG_SERIAL
    Serial.println("Peripherals -> sleep");
#endif
    // Turn off NeoPixel (send 'off', then release data pin)
    rgbPixel.clear();
    rgbPixel.show();
    // Cut Vext power (HIGH = OFF on CubeCell)
    digitalWrite(Vext, HIGH);

    // Put LoRa radio to sleep
    Radio.Sleep();

    // Set all unused GPIOs to INPUT (no pull) or analog to reduce leakage
    // (Example – adjust for your wiring; avoid WAKEUP_PIN)
    // pinMode(GPIO1, INPUT);
    // pinMode(GPIO2, INPUT);
}

// Helper: wake peripherals
static void exitPeripheralsSleep() {
    // Re‑enable Vext (LOW = ON)
    digitalWrite(Vext, LOW);
    delay(2);
    rgbPixel.begin();
    rgbPixel.clear();
    rgbPixel.show();
    // Radio will be reconfigured by your send routine as needed
#if DEBUG_SERIAL
    Serial.println("Peripherals -> wake");
#endif
}

// Minimal ISR (keep extremely short)
void rainTipISR() {
    rainTips++;
    lastRainTipTime = millis();
    // Just clear lowPowerMode flag and let main loop handle wake work
    lowPowerMode = 0;
}

// After wake processing (call once when lowPowerMode just cleared)
static void handleWake() {
    exitPeripheralsSleep();

#if DEBUG_SERIAL
    if (!serialReady) {
        Serial.begin(115200);
        delay(50);
        serialReady = true;
        Serial.println("Woke (handleWake).");
    }
#endif

    if (!bmeReady) {
        // Attempt re-init if power gated earlier
        bmeReady = bme.begin();
    }

    OwBmeReading reading{};
    if (bmeReady) {
        reading = bme.readAll();
#if DEBUG_SERIAL
        Serial.print("BME T=");
        Serial.print(reading.temperatureC, 2);
        Serial.print("C / ");
        Serial.print(reading.temperatureC * 9.0 / 5.0 + 32.0, 2);
        Serial.print("F ");
        Serial.print("C H=");
        Serial.print(reading.humidityPct, 2);
        Serial.print("% P=");
        Serial.print(reading.pressureHpa, 2);
        Serial.print("m Gas=");
        Serial.print(reading.gasKOhms, 3);
        Serial.println("kOhm");
#endif
    }

#if DEBUG_SERIAL
    Serial.print("rainTips=");
    Serial.println(rainTips);
#endif

    // Send combined weather packet over LoRa
    if (lora.initialized() && lora.idle()) {
        sendWeatherPacket(reading);
    } else {
#if DEBUG_SERIAL
        Serial.println("LoRa busy or not init; skip send");
#endif
    }

    TimerSetValue(&sleepTimer, TIMETILL_SLEEP);
    TimerStart(&sleepTimer);
}

// Function to handle sleep
void onSleep() {
#if DEBUG_SERIAL
    Serial.println("Entering low-power mode.");
    Serial.flush();
#endif
    lowPowerMode = 1;
    serialReady = false;

    enterPeripheralsSleep();

    // (Nothing else – lowPowerHandler() will drop MCU current)
}

void setup() {
#if DEBUG_SERIAL
    Serial.begin(115200);
    delay(200);
    Serial.println("Boot");
#endif

    pinMode(Vext, OUTPUT);
    digitalWrite(Vext, LOW);  // Power ON initially (Vext LOW = ON)

    rgbPixel.begin();
    rgbPixel.clear();
    rgbPixel.show();

    // Init BME680
    bmeReady = bme.begin();
#if DEBUG_SERIAL
    Serial.print("BME init: ");
    Serial.println(bmeReady ? "OK" : "FAIL");
#endif

    // Init LoRa
    lora.begin();

    pinMode(WAKEUP_PIN, INPUT_PULLUP);
    attachInterrupt(WAKEUP_PIN, rainTipISR, FALLING);

    TimerInit(&sleepTimer, onSleep);
    TimerSetValue(&sleepTimer, TIMETILL_SLEEP);
    TimerStart(&sleepTimer);

#if DEBUG_SERIAL
    Serial.println("Setup complete.");
#endif
}

void loop() {
    // If just woke (lowPowerMode was cleared by ISR), process wake tasks once
    static uint8_t lastLowPower = 1;
    if (lastLowPower == 1 && lowPowerMode == 0) {
        handleWake();
    }
    lastLowPower = lowPowerMode;

    if (lowPowerMode) {
        lowPowerHandler();  // enters deep sleep until interrupt / timer
    }

    // Optional small idle
    delay(1);
}