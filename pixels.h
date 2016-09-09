/*
 * pixels.h
 *
 * WS2812 RGB LED library for ReSpeaker (ATmega32U4)
 *
 * Copyright (c) 2016 Seeed Technology Limited.
 * MIT license
 *
 */

#ifndef __PIXELS_H_
#define __PIXELS_H_

#include <avr/interrupt.h>
#include <avr/io.h>
#ifndef F_CPU
#define  F_CPU 16000000UL
#endif
#include <util/delay.h>
#include <stdint.h>
#include <Arduino.h>

class Pixels {
public:
    Pixels(uint16_t num_leds);
    void begin(uint8_t pin);
    void set_color(uint32_t rgb);
    void set_color(uint16_t index, uint32_t rgb);
    void set_color(uint16_t index, uint8_t r, uint8_t g, uint8_t b);
    uint32_t get_color(uint16_t index);
    void set_brightness(uint8_t level);
    uint8_t get_brightness();
    void clear();
    void update();
    uint16_t number() {
        return leds_n;
    }

    static uint32_t RGB(uint8_t red, uint8_t green, uint8_t blue){
		return ((uint32_t)red << 16) | ((uint32_t)green << 8) | blue;
	};

    static uint32_t wheel(uint8_t position) {
        if(position < 85) {
            return RGB(position * 3, 255 - position * 3, 0);
        } else if(position < 170) {
            position -= 85;
            return RGB(255 - position * 3, 0, position * 3);
        } else {
            position -= 170;
            return RGB(0, position * 3, 255 - position * 3);
        }
    }

    void rainbow(uint8_t offset);

    void scroll(uint32_t px_value, uint8_t time);
    void scroll(uint8_t size, uint32_t px_value, uint8_t time);
    void pass_by(uint8_t size, uint32_t px_value1, uint32_t px_value2, uint8_t time);
    void blink(uint32_t px_value, uint8_t time, uint16_t index);
    void blink(uint32_t px_value, uint8_t time);
    void color_brush(uint32_t color, uint8_t time);
    void theater_chase(uint32_t color, uint8_t time);
    void theater_chase_rainbow(uint8_t time);

private:
    uint8_t *pixels;
    float brightness;
    uint16_t leds_n;
	uint32_t end_time;
    const volatile uint8_t *port;
    uint8_t pin_mask;
};

#endif /* __PIXELS_H_ */
