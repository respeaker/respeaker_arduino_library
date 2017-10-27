/*
   Please use Arduino IDE 1.6.8+

   Read battery voltage
*/

#include "respeaker.h"

#define PIXELS_SPACE    128

Pixels *pixels;
volatile uint32_t color;
volatile int pixels_state = -1;
volatile uint8_t volume = 0;
volatile uint32_t volume_changed_time = 0;


void spi_event(uint8_t addr, uint8_t *data, uint8_t len)
{
  if (0 == addr) {
    if (0 == data[0]) {   // all off
      color = 0x000000;
      pixels_state = 0;
    } else if (1 == data[0]) {  // specific color
      color = Pixels::RGB(data[3], data[2], data[1]);
      pixels_state = 0;
    } else if (2 == data[0] || 7 == data[0]) {    // listening mode, all green
      color = Pixels::RGB(0, 0x40, 0);
      pixels_state = 0;
    } else if (3 == data[0]) {    // waiting mode
      pixels_state = 1;
    }  else if (4 == data[0]) {   // speaking mode
      pixels_state = 2;
    } else if (5 == data[0]) {    // volume mode
      pixels_state = 3;
      volume = data[3];
      volume_changed_time = 0;
    }
  } else if (0xA0 == addr) {       // spectrum data
    static uint8_t current[PIXELS_NUM] = {0,};
    pixels_state = -1;
    Pixels pixels = respeaker.pixels();
    for (int i = 0; i < PIXELS_NUM; i++) {
      // Serial.print(data[i]);
      // Serial.print(' ');
      if (data[i] > current[i]) {
        current[i] = data[i];
      } else {
        current[i] = (data[i] + current[i]) >> 1;
      }
      pixels.set_color(i, change_brightness(b2r(i * 255 / (PIXELS_NUM - 1)), data[i]));
      // pixels.set_color(i, change_brightness(b2r(data[i]), data[i]));
    }
    pixels.update();
    // Serial.print("\r\n");
  }
}

void setup() {
  respeaker.begin();

  pixels = &respeaker.pixels();
  respeaker.attach_spi_handler(spi_event);

  pixels->set_brightness(10);

  pixels->rainbow(0);
  for (uint16_t i = 10; i <= 255; i++) {
    pixels->set_brightness(i);
    pixels->rainbow(0);
    pixels->update();
    delay(10);
  }

  for (uint16_t i = 255; i > 10; i--) {
    respeaker.pixels().set_brightness(i);
    pixels->rainbow(0);
    pixels->update();
    delay(10);
  }

  pixels->clear();
  pixels->update();

  pixels->set_brightness(255);

  pixels_state = 0;

  pinMode(A5, INPUT_PULLUP);
  pinMode(A1, INPUT);
}

uint8_t rainbow_offset = 0;

void loop() {
  static uint32_t last_time = 0;
  if (pixels_state == 0) {
    if (color == 0x004000) {
      pixels->rainbow(rainbow_offset++);
    } else {
      pixels->set_color(color);
      pixels->update();
    }
    pixels_state = -1;
  } else if (pixels_state == 1) {
    if ((uint32_t)(millis() - last_time) > 5) {
      last_time = millis();

      pixels->rainbow(rainbow_offset++);
    }

  } else if (pixels_state == 2) {
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
        pixels->set_color(i, 0, c, 0);
      }
      pixels->update();

      t += deta;
      if (t <= 0x40 || t >= 0xF0) {
        deta = -deta;
      }
    }
  } else if (pixels_state == 3) {
    if (0 == volume_changed_time) {
      uint8_t position = volume / 8;
      Serial.print("volume:");
      Serial.println(volume);
      for (int i = 0; i < PIXELS_NUM; i++) {
        uint8_t c = 0;
        if (i < position) {
          c = 0x20;
        }
        pixels->set_color(i, 0, c, 0);
      }
      pixels->update();
      volume_changed_time = millis();
    } else if ((uint32_t)(millis() - volume_changed_time) > 3000) {
      pixels_state = -1;
      pixels->clear();
      pixels->update();
    }
  } else {
    static uint32_t last = 0;
    if ((uint32_t)(millis() - last) > 2000) {
      last = millis();
      int16_t capacity = analogRead(A1) - 460;
      if (capacity > 255) {
        capacity = 255;
      } else if (capacity < 0) {
        capacity = 0;
      }
        
      pixels->clear();
      if (digitalRead(A5) == 0) {
        static uint8_t on = 0;
        on = 1 - on;
        if (!on) {
          pixels->set_color(0, change_brightness(b2r(255 - capacity / 2), 0x20));
        }
      } else {
        pixels->set_color(0, change_brightness(b2r(255 - capacity / 2), 0x20));
      }
      
      pixels->update();
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

uint32_t b2r(uint8_t position) {
  color_t color = {0};
  if (position < 128) {
    color.b = 255 - position * 2;
    color.g = position * 2;
  } else {
    color.g = 255 - position * 2;
    color.r = position * 2;
  }

  return color.c;
}

uint32_t change_brightness(uint32_t rgb, uint8_t level) {
  color_t color;
  color.c = rgb;
  color.r = color.r * level / 255;
  color.g = color.g * level / 255;
  color.b = color.b * level / 255;

  if (color.r < 4) {
    color.r = 0;
  }
  if (color.g < 4) {
    color.g = 0;
  }
  if (color.b < 4) {
    color.b = 0;
  }

  return color.c;
}

