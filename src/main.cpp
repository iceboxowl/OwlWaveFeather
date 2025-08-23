// Clean rebuilt main.cpp after corruption

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include <RH_RF69.h>
#include "ow_message_types.h"
#include "ow_controller.h"
#include "ow_global.h"

#if defined(ARDUINO_AVR_FEATHER32U4)
#include <EEPROM.h>
#endif

#include <Adafruit_SleepyDog.h>

// Debug / Power configuration
#define DEBUG_VERBOSE 1          // Set 0 to silence nearly all Serial prints
#define DEBUG_NO_SLEEP 0         // Set 1 to disable actual low power sleeps (for debugging timing)
#define DIAG_SKIP_BME 0          // Set 1 to skip BME sensor usage
#define SEND_INTERVAL_MS 20000UL // 20s cadence
// OW_LOWPOWER_DETACH_USB removed - USB detach optimization disabled

#if DEBUG_VERBOSE
    #define DBG_PRINT(x)   Serial.print(x)
    #define DBG_PRINTLN(x) Serial.println(x)
#else
    #define DBG_PRINT(x)
    #define DBG_PRINTLN(x)
#endif

// Track elapsed time toward next send using watchdog reported sleep (robust if millis pauses in standby)
static uint32_t elapsedSinceSendMs = 0;
static uint32_t lastActiveMillis = 0; // for diagnostic only

Adafruit_BME680 bme;
OwlWaveController owController;

// Forward declarations
//static void prepareForSleep();
//static void restoreAfterSleep();

void hallRainISM() { owController.onRainTip(); }

/**
 * @brief Puts the system into low-power sleep mode for a specified duration in milliseconds.
 *
 * This function attempts to sleep for the given number of milliseconds using the Watchdog timer,
 * accumulating the actual sleep duration. If debugging mode is enabled (DEBUG_NO_SLEEP), it uses
 * a simple delay instead. The radio is put to sleep before sleeping and woken up afterwards.
 *
 * @param ms The number of milliseconds to sleep. If zero, the function returns immediately.
 */
static void lowPowerSleepMs(uint32_t ms)
{
    if (ms == 0) return;
    if (DEBUG_NO_SLEEP) { delay(ms); elapsedSinceSendMs += ms; return; }

    owController.sleepRadio();

    uint32_t slept = 0;
    while (slept < ms)
    {
        uint32_t remaining = ms - slept;
        // SleepyDog caps internally (SAMD will enter standby between WDT interrupts)
        uint32_t actual = Watchdog.sleep(remaining);
        if (actual == 0) break; // safety
        slept += actual;
    }
    elapsedSinceSendMs += slept; // accumulate actual sleep duration

    owController.wakeRadio();
}

static void performSend()
{
    DBG_PRINTLN(F("-- Send Cycle --"));
    // Visual indicator: brief LED blink on Feather to show a send is occurring (only during debug)
#if DEBUG_VERBOSE
    digitalWrite(LED, HIGH);
    delay(60);
#endif
    DBG_PRINTLN(F("[SEND] wakeRadio"));
    owController.wakeRadio();
    DBG_PRINTLN(F("[SEND] calcRainRate"));
    owController.calculateRainRate();
    if (!DIAG_SKIP_BME)
    {
        DBG_PRINTLN(F("[SEND] BME performReading"));
        if (bme.performReading())
        {
            DBG_PRINTLN(F("[SEND] BME ok"));
            owController.sendBMEMessage(bme.temperature, bme.humidity, bme.pressure, bme.gas_resistance);
        }
        else
        {
            DBG_PRINTLN(F("[ERR] BME680 read fail"));
        }
        // Ensure gas heater is disabled between cycles to save power
        bme.setGasHeater(0,0);
    }
    DBG_PRINTLN(F("[SEND] sendRainMessage"));
    owController.sendRainMessage();
    DBG_PRINTLN(F("[SEND] sleepRadio"));
    owController.sleepRadio();
    DBG_PRINTLN(F("[SEND] done"));
#if DEBUG_VERBOSE
    digitalWrite(LED, LOW);
#endif
}

// Configure pins & peripherals for lowest practical consumption before sleeping
// static void prepareForSleep()
// {
//     // Turn LED off
//     pinMode(LED, OUTPUT);
//     digitalWrite(LED, LOW);

//     // Radio lines: keep CS high, RST low, INT input with pullup (match existing wiring expectations)
//     pinMode(RFM69_CS, OUTPUT); digitalWrite(RFM69_CS, HIGH);
//     pinMode(RFM69_RST, OUTPUT); digitalWrite(RFM69_RST, LOW);
//     pinMode(RFM69_INT, INPUT_PULLUP);

//     // (USB detach moved to loop() after grace period to avoid upload issues)

//     // Additional peripheral gating (SAMD21) – disable ADC if not used
// // #if defined(ARDUINO_ARCH_SAMD)
// //     ADC->CTRLA.bit.ENABLE = 0; while (ADC->STATUS.bit.SYNCBUSY); // disable ADC
// // #endif

//     // (no SPI/I2C shutdown optimizations enabled)
//     DBG_PRINTLN(F("[PREPARE] sleep completed"));
// }

// // Restore any peripherals after waking (only what we disabled explicitly)
// static void restoreAfterSleep()
// {
// #if defined(ARDUINO_ARCH_SAMD)
//     // Re-enable ADC for future sensor reads if BME (I2C) doesn't need it; leave disabled if not needed
//     // (BME680 over I2C doesn't use ADC; keep it off for power saving.)
// #endif
// }

void setup()
{
    Serial.begin(9600);
#if defined(ARDUINO_SAMD_ZERO) || defined(ARDUINO_SAMD_FEATHER_M0) || defined(ARDUINO_ARCH_SAMD)
    uint32_t start = millis();
    while (!Serial && (millis()-start)<4000) {}
#endif
    DBG_PRINTLN(F("Boot"));
    pinMode(LED, OUTPUT);
    pinMode(HALL_RAIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(HALL_RAIN), hallRainISM, FALLING);
    owController.setupRadio();

    if (!DIAG_SKIP_BME)
    {
        if (!bme.begin())
        {
            DBG_PRINTLN(F("[ERR] BME680 not found"));
        } else {
            bme.setTemperatureOversampling(BME680_OS_8X);
            bme.setHumidityOversampling(BME680_OS_2X);
            bme.setPressureOversampling(BME680_OS_4X);
            bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
            bme.setGasHeater(320, 150);
        }
    }
    performSend();
    elapsedSinceSendMs = 0;
    lastActiveMillis = millis();
}

void loop()
{
    // If not using DEBUG_NO_SLEEP, millis may not advance during deep standby on SAMD.
    // Use elapsedSinceSendMs (watchdog-based) as primary scheduler.

    if (elapsedSinceSendMs >= SEND_INTERVAL_MS)
    {
        DBG_PRINTLN(F("=== Cycle ==="));
        performSend();
        elapsedSinceSendMs = 0;
        lastActiveMillis = millis();
    }

    uint32_t remaining = (elapsedSinceSendMs < SEND_INTERVAL_MS) ? (SEND_INTERVAL_MS - elapsedSinceSendMs) : 0;
    lowPowerSleepMs(remaining);
}