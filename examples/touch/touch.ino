/*
   Please use Arduino IDE 1.6.8+

   Get touch with Touch
   Use 8 capacitive touch sensors to trigger onboard LEDs
*/

#include "respeaker.h"


void setup() {
  respeaker.begin();
  respeaker.attach_touch_handler(touch_event);  // add touch event handler
}

void loop() {}

// id: 0 ~ 7 - touch sensor id; event: 1 - touch, 0 - release
void touch_event(uint8_t id, uint8_t event) {
  if (event) {
    respeaker.pixels().set_color(id, Pixels::wheel(id * 32));
  } else {
    respeaker.pixels().set_color(id, 0);
  }
  respeaker.pixels().update();
}
