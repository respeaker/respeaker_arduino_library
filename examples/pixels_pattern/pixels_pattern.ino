/*
   Please use Arduino IDE 1.6.8+

   receive spi commands to change pixels pattern
*/

#include "respeaker.h"

#define PIXELS_SPACE    128

Pixels *pixels;
int pixels_state = 0;

const char *pixels_patterns[] = {"sleep", "wakeup", "wait", "answer", "offline", "online"};

void touch_event(uint8_t id, uint8_t event) {

}

void spi_event(uint8_t addr, uint8_t *data, uint8_t len)
{
  for (uint8_t i = 0; i < sizeof(pixels_patterns) / sizeof(*pixels_patterns); i++) {
    if (!strcmp(pixels_patterns[i], (char *)data)) {
      pixels_state = i;
      break;
    }
  }
}

void setup() {
  respeaker.begin();
  respeaker.attach_touch_handler(touch_event);
  respeaker.attach_spi_handler(spi_event);

  pixels = &respeaker.pixels();
  for (int i = 0; i < PIXELS_NUM; i++) {
    pixels->set_pixel(i, 0, 0, 32);
  }
  pixels->update();

  delay(1000);
  pixels->clear();
  pixels->update();

  pixels_state = 3;
}

void loop() {
  static uint32_t last_time = 0;

  if (pixels_state == 0) {
    pixels->clear();
    pixels->update();
    pixels_state = -1;
  } else if (pixels_state == 1) {
    for (int i = 0; i < PIXELS_NUM; i++) {
      pixels->set_pixel(i, 0, 0xFF, 0);
    }
    pixels->update();
    pixels_state = -1;
  } else if (pixels_state == 2) {
    static uint32_t t = 0;
    for (int i = 0; i < PIXELS_NUM; i++) {
      pixels->set_pixel(i, triangular_color((t + i * PIXELS_SPACE) % (PIXELS_SPACE * PIXELS_NUM)));
    }
    pixels->update();

    t++;
    if (t >= (PIXELS_SPACE * PIXELS_NUM)) {
      t = 0;
    }
  } else if (pixels_state == 3) {
    static uint32_t arc[PIXELS_NUM] = {0x10, 0x20, 0x30, 0x50, 0x80, 0xC0, 0xFF, 0xC0, 0x80, 0x50, 0x30, 0x20};
    static uint8_t t = 0x80;
    static int8_t deta = 1;

    uint32_t current = millis();
    if ((uint32_t)(current - last_time) > 5) {
      last_time = current;
      for (int i = 0; i < PIXELS_NUM; i++) {
        int16_t c = arc[i] - t;
        if (c < 0) {
          c = 0;
        }
        pixels->set_pixel(i, 0, c, 0);
      }
      pixels->update();

      t += deta;
      if (t <= 0x40 || t >= 0xF0) {
        deta = -deta;
      }
    }
  } else if (pixels_state == 4) {
    for (int i = 0; i < PIXELS_NUM; i++) {
      pixels->set_pixel(i, 0xFF, 0, 0);
    }
    pixels->update();
    pixels_state = -1;
  } else if (pixels_state == 5) {
    static uint8_t on = 0;

    if (!on) {
      for (int i = 0; i < PIXELS_NUM; i++) {
        pixels->set_pixel(i, 0, 0xFF, 0);
      }
      pixels->update();
      on = 1;
      last_time = millis();
    } else {
      uint32_t current = millis();
      if ((uint32_t)(current - last_time) >= 1000) {
        for (int i = 0; i < PIXELS_NUM; i++) {
          pixels->set_pixel(i, 0, 0, 0);
        }
        pixels->update();

        on = 0;
        pixels_state = -1;
      }
    }
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