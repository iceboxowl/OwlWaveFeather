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
#endif

// Singleton instance of the radio driver
RH_RF69 rf69(RFM69_CS, RFM69_INT);

int16_t packetnum = 0;  // packet counter, we increment per xmission

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME680 bme; // I2C
OwlWaveController owController;

void setup() 
{
    Serial.begin(9600);
    Serial.println(F("BME680 test"));

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

    Serial.println("Feather RFM69 TX Test!");
    Serial.println();

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
    
    pinMode(LED, OUTPUT);

    Serial.print("RFM69 radio @");  Serial.print((int)RF69_FREQ);  Serial.println(" MHz");
}

void loop() 
{
    if (! bme.performReading()) 
    {
      Serial.println("Failed to perform reading :(");
      return;
    }

    delay(20000);  // Wait 20 seconds between transmits, could also 'sleep' here!

    // Message Types
    String messageTypeTemperature = String(OwlWaveMessageTypes::TEMPERATURE) + "=";
    String messageTypeHumidity = String(OwlWaveMessageTypes::HUMIDITY) + "=";
    String messageTypePressure = String(OwlWaveMessageTypes::PRESSURE) + "=";
    String messageTypeGas = String(OwlWaveMessageTypes::GAS) + "=";
    String messageSep = String(OW_MESSAGE_SEPERATOR);

    char buffer_msgTemperature[3];
    char buffer_msgHumidity[3];
    char buffer_msgPressure[3];
    char buffer_msgGas[3];
    char buffer_sep[2];

    messageSep.toCharArray(buffer_sep, 2);
    messageTypeTemperature.toCharArray(buffer_msgTemperature, 3);
    messageTypeHumidity.toCharArray(buffer_msgHumidity, 3);
    messageTypePressure.toCharArray(buffer_msgPressure, 3);
    messageTypeGas.toCharArray(buffer_msgGas, 3);

    char buffer_temperature[6];
    char buffer_humidity[6];
    char buffer_pressure[6];
    char buffer_gas[6];
    char buffer[46] = "";
 
    strcat(buffer, buffer_msgTemperature);
    //4 is mininum width, 2 is precision; float value is copied onto buff
    dtostrf(bme.temperature, 4, 2, buffer_temperature); // C
    strcat(buffer, buffer_temperature);
    strcat(buffer, buffer_sep);

    strcat(buffer, buffer_msgHumidity);
    dtostrf(bme.humidity, 4, 2, buffer_humidity); //%
    strcat(buffer, buffer_humidity);
    strcat(buffer, buffer_sep);
    
    strcat(buffer, buffer_msgPressure);
    dtostrf((bme.pressure / 100.0), 6, 2, buffer_pressure); //hPa
    strcat(buffer, buffer_pressure);
    strcat(buffer, buffer_sep);
    
    strcat(buffer, buffer_msgGas);
    dtostrf((bme.gas_resistance / 1000.0), 6, 2, buffer_gas); //KOhms
    strcat(buffer, buffer_gas);

    Serial.print("Sending "); Serial.println(buffer);
    
    // Send a message!
    rf69.send((uint8_t *)buffer, strlen(buffer));
    rf69.waitPacketSent();
    owController.blink(LED, 50, 3);

    // Now wait for a reply
    uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    if (rf69.waitAvailableTimeout(500))  
    { 
        // Should be a reply message for us now   
        if (rf69.recv(buf, &len)) 
        {
          Serial.print("Got a reply: ");
          Serial.println((char*)buf);
          //Blink(LED, 50, 3); //blink LED 3 times, 50ms between blinks
        } 
        else 
        {
          Serial.println("Receive failed");
        }
    } 
    else 
    {
      Serial.println("No reply, is another RFM69 listening?");
    }

    delay(2000);
}