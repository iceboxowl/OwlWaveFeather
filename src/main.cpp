#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include <RH_RF69.h>
#include "ow_message_types.h"
#include "ow_controller.h"

/************ Radio Setup ***************/

// Change to 434.0 or other frequency, must match RX's freq!
#define RF69_FREQ 915.0

#if defined (__AVR_ATmega32U4__) // Feather 32u4 w/Radio
  #define RFM69_CS      8
  #define RFM69_INT     7
  #define RFM69_RST     4
  #define LED           13
  #define HALLRAIN      1
#endif

// Singleton instance of the radio driver
RH_RF69 rf69(RFM69_CS, RFM69_INT);

int16_t packetnum = 0;  // packet counter, we increment per xmission
int32_t rainTips = 0;

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

Adafruit_BME680 bme; // I2C
OwlWaveController owController;

void hallRainTipISM()
{
    rainTips++;
}

void setup() 
{
    Serial.begin(9600);
    Serial.println(F("BME680 test"));

    pinMode(HALLRAIN, INPUT_PULLUP);
    pinMode(LED, OUTPUT);
    attachInterrupt(digitalPinToInterrupt(HALLRAIN), hallRainTipISM, RISING);

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
    
    if (!rf69.init()) 
    {
      Serial.println("RFM69 radio init failed");
      while (1);
    }
    Serial.println("RFM69 radio init OK!");
    // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM (for low power module)
    // No encryption
    if (!rf69.setFrequency(RF69_FREQ)) 
    {
      Serial.println("setFrequency failed");
    }

    // If you are using a high power RF69 eg RFM69HW, you *must* set a Tx power with the
    // ishighpowermodule flag set like this:
    rf69.setTxPower(20, true);  // range from 14-20 for power, 2nd arg must be true for 69HCW

    // The encryption key has to be the same as the one in the server
    uint8_t key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                      0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    rf69.setEncryptionKey(key);

    Serial.print("RFM69 radio @");  Serial.print((int)RF69_FREQ);  Serial.println(" MHz");
}

void loop() 
{
    if (! bme.performReading()) 
    {
        Serial.println("Failed to perform reading :(");
        return;
    }

    delay(2000);  // Wait 20 seconds between transmits, could also 'sleep' here!

    char buffer[46] = ""; 
    owController.getBMEMessage(bme.temperature, bme.humidity, bme.pressure, bme.gas_resistance).toCharArray(buffer, 46);
    
    Serial.print("Sending "); Serial.println(buffer);
    Serial.print("Rain Tips "); Serial.println(rainTips);
    // Send a message!
    rf69.send((uint8_t *)buffer, strlen(buffer));
    rf69.waitPacketSent();
    owController.blink(LED, 50, 3);

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
}