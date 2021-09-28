#ifndef OWLWAVE_GLOBAL_H
#define OWLWAVE_GLOBAL_H

#define RFM69_CS        10      // "B"
#define RFM69_RST       11      // "A"
#define RFM69_INT       6       // "D"
#define LED             13
#define HALL_RAIN       1

// Change to 434.0 or other frequency, must match RX's freq!
#define RF69_FREQ 915.69
#define RAIN_TIP_AMOUNT 0.01 // Each rain tip is equal to this amount.

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define MAX_LOOP_CYCLE 20 // How many loops(1s for each loop) before sending temperature message.

#endif //OWLWAVE_GLOBAL_H