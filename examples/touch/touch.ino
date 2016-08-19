/*
   Please use Arduino IDE 1.6.8+

   Get touch with Touch
   Use 8 capacitive touch sensors to interact with human being
*/

#include "respeaker.h"

// id - touch sensor id (0 ~ 7), event - 1: touch, 0: release
void touch_event(uint8_t id, uint8_t event) {
  if (event) {
    respeaker.pixels().set_pixel(id + 2, Pixels::wheel(id * 32));
  } else {
    respeaker.pixels().set_pixel(id + 2, 0);
  }
  respeaker.pixels().update();
}

void setup() {
  respeaker.begin();
  respeaker.attach_touch_handler(touch_event);  // add touch event handler
}

void loop() {

}
