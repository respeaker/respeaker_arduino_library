/*
   Please use Arduino IDE 1.6.8+ which has some useful new features
*/

#include "respeaker.h"

#define PIXELS_SPACE        128

uint32_t t = 0;

void touch_event(uint8_t id, uint8_t event) {
    // id - touch sensor id (0 ~ 7), event - 1: touch, 0: release
}

void setup() {
  respeaker.begin();
  respeaker.attach_touch_handler(touch_event);
}

void loop() {
  for (int i = 0; i < PIXELS_NUM; i++) {
    respeaker.pixels().set_color(i, triangular_color((t + i * PIXELS_SPACE) % (PIXELS_SPACE * PIXELS_NUM)));
  }
  respeaker.pixels().update();

  t++;
  if (t >= (PIXELS_SPACE * PIXELS_NUM)) {
    t = 0;
  }
}

uint32_t triangular_color(uint32_t t) {
  uint32_t c = 0;

  if (t < 256) {
    c = Pixels::RGB(0, t, 0);
  } else if (t < 512) {
    c = Pixels::RGB(0, 511 - t, 0);
  }

  return c;
}