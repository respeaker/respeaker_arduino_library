/*
   Please use Arduino IDE 1.6.8+

   Be colorful!
*/

#include "respeaker.h"

uint8_t offset = 0;

void setup() {
  respeaker.begin();
  respeaker.pixels().set_brightness(0x40);      // set brightness level (from 0 to 0xFF)
}

void loop() {
  respeaker.pixels().rainbow(offset++);
  delay(10);
}
