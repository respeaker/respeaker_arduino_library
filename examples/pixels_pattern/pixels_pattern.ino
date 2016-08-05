
#include "SPI.h"
#include "respeaker.h"
#include <Adafruit_NeoPixel.h>
// #include "Streaming.h"

#define PIXELS_PIN      11
#define PIXELS_NUM      12
#define PIXELS_SPACE    128

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(PIXELS_NUM, PIXELS_PIN, NEO_GRB + NEO_KHZ800);
int pixels_state = 0;

const char *pixels_patterns[] = {"sleep", "wakeup", "wait", "answer", "offline", "online"};

void touch_event(uint8_t id, uint8_t event) {
  //  Serial << "id:" << id << " event:" << event << "\r\n";
}

void spi_event(uint8_t addr, uint8_t *data, uint8_t len)
{
  for (uint8_t i = 0; i < sizeof(pixels_patterns) / sizeof(*pixels_patterns); i++) {
    if (!strcmp(pixels_patterns[i], (char *)data)) {
      pixels_state = i;
      break;
    }
  }
  
  // Serial << "spi rx:" << (char*)data << " state: " << pixels_state << "\r\n";
}

void setup() {
  pixels.begin();
  for (int i = 0; i < PIXELS_NUM; i++) {
    pixels.setPixelColor(i, 0, 0, 32);
  }
  pixels.show();

  respeaker.begin();
  respeaker.attach_touch_isr(touch_event);
  respeaker.attach_spi_isr(spi_event);

  delay(1000);
  pixels.clear();
  pixels.show();

//  pixels_state = 3;
}

void loop() {
  static uint32_t last_time = 0;

  if (pixels_state == 0) {
    pixels.clear();
    pixels.show();
    pixels_state = -1;
  } else if (pixels_state == 1) {
    for (int i = 0; i < PIXELS_NUM; i++) {
      pixels.setPixelColor(i, 0, 0xFF, 0);
    }
    pixels.show();
    pixels_state = -1;
  } else if (pixels_state == 2) {
    static uint32_t t = 0;
    for (int i = 0; i < PIXELS_NUM; i++) {
      pixels.setPixelColor(i, triangular_color((t + i * PIXELS_SPACE) % (PIXELS_SPACE * PIXELS_NUM)));
    }
    pixels.show();

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
        pixels.setPixelColor(i, 0, c, 0);
      }
      pixels.show();
  
      t += deta;
      if (t <= 0x40 || t >= 0xF0) {
        deta = -deta;
      }
    }
  } else if (pixels_state == 4) {
    for (int i = 0; i < PIXELS_NUM; i++) {
      pixels.setPixelColor(i, 0xFF, 0, 0);
    }
    pixels.show();
    pixels_state = -1;
  } else if (pixels_state == 5) {
    static uint8_t on = 0;

    if (!on) {
      for (int i = 0; i < PIXELS_NUM; i++) {
        pixels.setPixelColor(i, 0, 0xFF, 0);
      }
      pixels.show();
      on = 1;
      last_time = millis();
    } else {
      uint32_t current = millis();
      if ((uint32_t)(current - last_time) >= 1000) {
        for (int i = 0; i < PIXELS_NUM; i++) {
          pixels.setPixelColor(i, 0, 0, 0);
        }
        pixels.show();

        on = 0;
        pixels_state = -1;
      }
    }
  }
}

uint32_t triangular_color(uint32_t t)
{
  uint32_t c;

  if (t < 256) {
    c = pixels.Color(0, t, 0);
  } else if (t < 512) {
    c = pixels.Color(0, 511 - t, 0);
  }

  return c;
}
