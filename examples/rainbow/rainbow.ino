/*
   Please use Arduino IDE 1.6.8+ which has some useful new features

   Chasing colors!
*/

#include "respeaker.h"

uint8_t offset = 0;

void setup() {
  respeaker.begin();
  respeaker.pixels().set_brightness(128);      // set brightness level (from 0 to 255)
}

void loop() {
  respeaker.pixels().rainbow(offset++);
  delay(10);
}
