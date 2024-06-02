#include <Arduino.h> // for esp32
#include <HardwareSerial.h>

#include "Adafruit_Thermal.h"
#include "test.h" // img exemple
#include "adalogo.h"
#include "adaqrcode.h"

#define TX_PIN 1 // esp32 transmit  YELLOW WIRE  labeled RX on printer
#define RX_PIN 3 // esp32 receive   GREEN WIRE   labeled TX on printer

// #define TX_PIN 14
// #define RX_PIN 2

HardwareSerial printerSerial(1);
Adafruit_Thermal printer(&printerSerial); // Pass addr to printer constructor

// forward declaration:
void testImg(void);
void pTutTest(void);
void test(void);

void setup() {
  printerSerial.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);

  printer.begin();
  delay(500);
  
  testImg();
  delay(2000);
  test();
  delay(2000);
  pTutTest();
  delay(2000);
  testImg();
}


void loop() {
}

void pTutTest() {
   printer.justify('C');
   printer.setSize('S');
   printer.println(F("----------"));
   printer.setSize('L');
   printer.println(F("Ptut"));
   printer.setSize('S');
   printer.println(F("----------"));
   printer.sleep();      // Tell printer to sleep
   delay(1000L);         // Sleep for 3 seconds
   printer.wake();       // MUST wake() before printing again, even if reset
   printer.setDefault(); // Restore printer to defaults
}

void testImg() {
  printer.printBitmap(test_width, test_height, test_data);
  printer.sleep();      // Tell printer to sleep
  delay(3000L);         // Sleep for 3 seconds
  printer.wake();       // MUST wake() before printing again, even if reset
  printer.setDefault(); // Restore printer to defaults
}

void test() {
  for (size_t i = 0; i < 3; i++) {
    printer.printBitmap(adaqrcode_width, adaqrcode_height, adaqrcode_data);
    printer.println(F("---test---"));
  }

  printer.sleep();      // Tell printer to sleep
  delay(3000L);         // Sleep for 3 seconds
  printer.wake();       // MUST wake() before printing again, even if reset
  printer.setDefault(); // Restore printer to defaults
}