#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include <RH_RF69.h>
#include "ow_message_types.h"
#include "ow_controller.h"
#include "ow_global.h"

// -*-*-*-*-*-*-*-*-*-*-*-*-*-* //
//      Class Creation          //
// -*-*-*-*-*-*-*-*-*-*-*-*-*-* //
Adafruit_BME680 bme; // I2C
OwlWaveController owController; // For handling all logic
// -*-*-*-*-*-*-*-*-*-*-*-*-*-* //

uint8_t loopCount;      // How many times our loop iteration has run, within our MAX_LOOP_CYLE.
uint32_t upTime;        // How many seconds our device has been on for.

// ISM's must be defined here, but can make calls into our Controller class.
void hallRainISM()
{
    owController.onRainTip();
}

void setup() 
{
    Serial.begin(9600);
    Serial.println(F("BME680 test"));
    
    pinMode(LED, OUTPUT);
    pinMode(HALL_RAIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(HALL_RAIN), hallRainISM, RISING);

    owController.setupRadio();

    if (!bme.begin()) 
    {
      Serial.println("Could not find a valid BME680 sensor, check wiring!");
      while (1);
    }

    // Set up oversampling and filter initialization
    bme.setTemperatureOversampling(BME680_OS_8X);
    bme.setHumidityOversampling(BME680_OS_2X);
    bme.setPressureOversampling(BME680_OS_4X);
    bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
    bme.setGasHeater(320, 150); // 320*C for 150 ms

    // manual reset
    digitalWrite(RFM69_RST, HIGH);
    delay(10);
    digitalWrite(RFM69_RST, LOW);
    delay(10);
}

void loop() 
{
    loopCount++;

    // If loopCount(Seconds) has surpassed our MAX_LOOP_CYCLE send our bme message.
    if (loopCount > MAX_LOOP_CYCLE)
    {
        Serial.print("Up Time "); Serial.print(upTime); Serial.println("s");
        owController.calculateRainRate();

        if (bme.performReading()) // Check to make sure our reading succeeds before sending.
        {
            owController.sendBMEMessage(bme.temperature, bme.humidity, bme.pressure, bme.gas_resistance);
        }
        else
        {
            Serial.println("Failed to perform reading :(");
        }

        owController.sendRainMessage();
        loopCount = 0;
    }

    upTime++;
    delay(1000); // Pause so our loop is done every second.

    // Send a message!
    // rf69.send((uint8_t *)buffer, strlen(buffer));
    // rf69.waitPacketSent();
    // owController.blink(LED, 50, 3);

    // // Now wait for a reply
    // uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
    // uint8_t len = sizeof(buf);

    // if (rf69.waitAvailableTimeout(500))  
    // { 
    //     // Should be a reply message for us now   
    //     if (rf69.recv(buf, &len)) 
    //     {
    //       Serial.print("Got a reply: ");
    //       Serial.println((char*)buf);
    //       //Blink(LED, 50, 3); //blink LED 3 times, 50ms between blinks
    //     } 
    //     else 
    //     {
    //       Serial.println("Receive failed");
    //     }
    // } 
    // else 
    // {
    //   Serial.println("No reply, is another RFM69 listening?");
    // }

      // Wait 20 seconds between transmits, could also 'sleep' here!
}