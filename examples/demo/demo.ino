
#include "SPI.h"
#include "respeaker.h"
#include <Adafruit_NeoPixel.h>

#define PIXELS_PIN      11
#define PIXELS_NUM      12
#define PIXELS_SPACE    128

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(PIXELS_NUM, PIXELS_PIN, NEO_GRB + NEO_KHZ800);

void touch_event(uint8_t id, uint8_t event) {
  
}

void spi_event(uint8_t addr, uint8_t *data, uint8_t len)
{

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
}

void loop() {
  static uint32_t t = 0;
  for (int i = 0; i < PIXELS_NUM; i++) {
    pixels.setPixelColor(i, triangular_color((t + i * PIXELS_SPACE) % (PIXELS_SPACE * PIXELS_NUM)));
  }
  pixels.show();

  t++;
  if (t >= (PIXELS_SPACE * PIXELS_NUM)) {
    t = 0;
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
