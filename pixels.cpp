/*
 * pixels.cpp
 *
 * WS2812 RGB LED library for ReSpeaker (ATmega32U4)
 *
 * Copyright (c) 2016 Seeed Technology Limited.
 * MIT license
 *
 */

#include "pixels.h"
#include <stdlib.h>

Pixels::Pixels(uint16_t num_leds)
{
    leds_n = num_leds;
    pixels = new uint8_t[leds_n * 3];
    brightness = 1.0;
    clear();
}

void Pixels::begin(uint8_t pin)
{
	pinMode(pin, OUTPUT);
	digitalWrite(pin, LOW);
    pin_mask = digitalPinToBitMask(pin);
    port = portOutputRegister(digitalPinToPort(pin));
}

void Pixels::set_color(uint32_t rgb)
{
    uint8_t r = ((rgb >> 16) & 0xff) * brightness;
    uint8_t g = ((rgb >> 8) & 0xff) * brightness;
    uint8_t b = (rgb & 0xff) * brightness;
    uint8_t *ptr = pixels;
    uint16_t index = 0;
    while (index < leds_n) {
        *ptr++ = g;
        *ptr++ = r;
        *ptr++ = b;
        index++;
    }
}

void Pixels::set_color(uint16_t index, uint32_t rgb)
{
    if(index < leds_n) {
        uint8_t *ptr = pixels + index * 3;
        *ptr++ = ((rgb >> 8) & 0xff) * brightness;     // green
        *ptr++ = ((rgb >> 16) & 0xff) * brightness;    // red
        *ptr   = (rgb & 0xff) * brightness;            // blue
    }
}

void Pixels::set_color(uint16_t index, uint8_t r, uint8_t g, uint8_t b)
{
    if(index < leds_n) {
        uint8_t *ptr = pixels + index * 3;
        *ptr++ = g * brightness;  // green
        *ptr++ = r * brightness;  // red
        *ptr   = b * brightness;  // blue
    }
}

uint32_t Pixels::get_color(uint16_t index)
{
    uint8_t *ptr = pixels + index * 3;

    return RGB(ptr[1], ptr[0], ptr[2]);
}

void Pixels::set_brightness(uint8_t level)
{
    brightness = level / 255.0;
}

uint8_t Pixels::get_brightness()
{
    return brightness * 255;
}

void Pixels::clear()
{
    memset(pixels, 0, leds_n * 3);
    update();
}

void Pixels::scroll(uint32_t px_value, uint8_t time)
{
    for(uint16_t i = 0; i < leds_n; i++){
        clear();
        set_color(i, px_value);
        update();
        delay(time);
    }
}

void Pixels::scroll(uint8_t size, uint32_t px_value, uint8_t time)
{
    if(size > leds_n){
        return;
    }
    for(int i = 1; i < leds_n + size; i++){
        if(i < size){
            for(int j = 0; j < i; j++){
                set_color(j, px_value);
            }
        }else if(i >= leds_n){
            for(int k = i - size; k < leds_n; k++){
                set_color(k, px_value);
            }
        }else{
            for(int m = i-size; m < i; m++){
                set_color(m, px_value);
            }
        }
        update();
        delay(time);
        clear();
    }

}

void Pixels::pass_by(uint8_t size, uint32_t px_value1, uint32_t px_value2, uint8_t time)
{
    if(size > leds_n){
        return;
    }
    for(int i = 1; i < leds_n + size; i++){
        if(i < size){
            for(int j = 0; j < i; j++){
                set_color(j, px_value1);
                set_color(leds_n -1 - j, px_value2);
            }
        }else if(i >= leds_n){
            for(int k = i - size; k < leds_n; k++){
                set_color(k, px_value1);
                set_color(leds_n -1 - k, px_value2);
            }
        }else{
            for(int m = i-size; m < i; m++){
                set_color(m, px_value1);
                set_color(leds_n -1 - m, px_value2);
            }
        }
        update();
        delay(time);
        clear();
    }
}

void Pixels::blink(uint32_t px_value, uint8_t time, uint16_t index)
{
    set_color(index, px_value);
    update();
    delay(time);
    set_color(index, 0);
    update();
    delay(time);
}

void Pixels::blink(uint32_t px_value, uint8_t time)
{
    for(int i = 0; i < leds_n; i++){
        set_color(i, px_value);
    }
    update();
    delay(time);
    clear();
    delay(time);
}

void Pixels::color_brush(uint32_t px_value, uint8_t time)
{
    for(uint16_t index = 0; index < leds_n; index++) {
        set_color(index, px_value);
        update();
        delay(time);
    }
}

void Pixels::rainbow(uint8_t offset)
{
    uint16_t i;
    for(i = 0; i < leds_n; i++) {
        set_color(i, wheel(((i * 256 / leds_n) + offset) & 255));
    }
    update();
}

void Pixels::theater_chase(uint32_t px_value, uint8_t time)
{
    for (int j = 0; j < 10; j++) {
        for (int q = 0; q < 3; q++) {
            for (int i = 0; i < leds_n; i = i+3) {
                set_color(i+q, px_value);
            }
            update();
            delay(time);
            for (int i = 0; i < leds_n; i = i+3) {
                set_color(i+q, 0);
            }
        }
    }
}

void Pixels::theater_chase_rainbow(uint8_t time)
{
    for (int j = 0; j < 256; j++) {
        for (int q = 0; q < 3; q++) {
            for (int i = 0; i < leds_n; i=i+3) {
                set_color(i+q, wheel( (i+j) % 255));
            }
            update();
            delay(time);
            for (int i = 0; i < leds_n; i = i+3) {
                set_color(i+q, 0);
            }
        }
    }
}

void Pixels::update()
{
	if ((uint32_t)(micros() - end_time) < 50) { return; }

	noInterrupts();

	volatile uint16_t i = leds_n * 3;
	volatile uint8_t *ptr = pixels;
	volatile uint8_t b = *ptr++;
	volatile uint8_t hi;
	volatile uint8_t lo;

#if (F_CPU >= 7400000UL) && (F_CPU <= 9500000UL)
    volatile uint8_t n1, n2 = 0;
    if(port == &PORTD){
		hi = PORTD |  pin_mask;
		lo = PORTD & ~pin_mask;
		n1 = lo;
		if(b & 0x80) n1 = hi;
		asm volatile(
        "headD:"                   "\n\t"
        // Bit 7:
        "out  %[port] , %[hi]"    "\n\t"
        "mov  %[n2]   , %[lo]"    "\n\t"
        "out  %[port] , %[n1]"    "\n\t"
        "rjmp .+0"                "\n\t"
        "sbrc %[byte] , 6"        "\n\t"
         "mov %[n2]   , %[hi]"    "\n\t"
        "out  %[port] , %[lo]"    "\n\t"
        "rjmp .+0"                "\n\t"
        // Bit 6:
        "out  %[port] , %[hi]"    "\n\t"
        "mov  %[n1]   , %[lo]"    "\n\t"
        "out  %[port] , %[n2]"    "\n\t"
        "rjmp .+0"                "\n\t"
        "sbrc %[byte] , 5"        "\n\t"
         "mov %[n1]   , %[hi]"    "\n\t"
        "out  %[port] , %[lo]"    "\n\t"
        "rjmp .+0"                "\n\t"
        // Bit 5:
        "out  %[port] , %[hi]"    "\n\t"
        "mov  %[n2]   , %[lo]"    "\n\t"
        "out  %[port] , %[n1]"    "\n\t"
        "rjmp .+0"                "\n\t"
        "sbrc %[byte] , 4"        "\n\t"
         "mov %[n2]   , %[hi]"    "\n\t"
        "out  %[port] , %[lo]"    "\n\t"
        "rjmp .+0"                "\n\t"
        // Bit 4:
        "out  %[port] , %[hi]"    "\n\t"
        "mov  %[n1]   , %[lo]"    "\n\t"
        "out  %[port] , %[n2]"    "\n\t"
        "rjmp .+0"                "\n\t"
        "sbrc %[byte] , 3"        "\n\t"
         "mov %[n1]   , %[hi]"    "\n\t"
        "out  %[port] , %[lo]"    "\n\t"
        "rjmp .+0"                "\n\t"
        // Bit 3:
        "out  %[port] , %[hi]"    "\n\t"
        "mov  %[n2]   , %[lo]"    "\n\t"
        "out  %[port] , %[n1]"    "\n\t"
        "rjmp .+0"                "\n\t"
        "sbrc %[byte] , 2"        "\n\t"
         "mov %[n2]   , %[hi]"    "\n\t"
        "out  %[port] , %[lo]"    "\n\t"
        "rjmp .+0"                "\n\t"
        // Bit 2:
        "out  %[port] , %[hi]"    "\n\t"
        "mov  %[n1]   , %[lo]"    "\n\t"
        "out  %[port] , %[n2]"    "\n\t"
        "rjmp .+0"                "\n\t"
        "sbrc %[byte] , 1"        "\n\t"
         "mov %[n1]   , %[hi]"    "\n\t"
        "out  %[port] , %[lo]"    "\n\t"
        "rjmp .+0"                "\n\t"
        // Bit 1:
        "out  %[port] , %[hi]"    "\n\t"
        "mov  %[n2]   , %[lo]"    "\n\t"
        "out  %[port] , %[n1]"    "\n\t"
        "rjmp .+0"                "\n\t"
        "sbrc %[byte] , 0"        "\n\t"
         "mov %[n2]   , %[hi]"    "\n\t"
        "out  %[port] , %[lo]"    "\n\t"
        "sbiw %[count], 1"        "\n\t"
        // Bit 0:
        "out  %[port] , %[hi]"    "\n\t"
        "mov  %[n1]   , %[lo]"    "\n\t"
        "out  %[port] , %[n2]"    "\n\t"
        "ld   %[byte] , %a[ptr]+" "\n\t"
        "sbrc %[byte] , 7"        "\n\t"
         "mov %[n1]   , %[hi]"    "\n\t"
        "out  %[port] , %[lo]"    "\n\t"
        "brne headD"              "\n"
      : [byte]  "+r" (b),
        [n1]    "+r" (n1),
        [n2]    "+r" (n2),
        [count] "+w" (i)
      : [port]   "I" (_SFR_IO_ADDR(PORTD)),
        [ptr]    "e" (ptr),
        [hi]     "r" (hi),
        [lo]     "r" (lo));
    }
#elif (F_CPU >= 11100000UL) && (F_CPU <= 14300000UL)
    volatile uint8_t next;
    if(port == &PORTD) {
      hi   = PORTD |  pin_mask;
      lo   = PORTD & ~pin_mask;
      next = lo;
      if(b & 0x80) next = hi;
      asm volatile(
        "headD:"                   "\n\t"
        "out   %[port], %[hi]"    "\n\t"
        "rcall bitTimeD"          "\n\t"
        "out   %[port], %[hi]"    "\n\t"
        "rcall bitTimeD"          "\n\t"
        "out   %[port], %[hi]"    "\n\t"
        "rcall bitTimeD"          "\n\t"
        "out   %[port], %[hi]"    "\n\t"
        "rcall bitTimeD"          "\n\t"
        "out   %[port], %[hi]"    "\n\t"
        "rcall bitTimeD"          "\n\t"
        "out   %[port], %[hi]"    "\n\t"
        "rcall bitTimeD"          "\n\t"
        "out   %[port], %[hi]"    "\n\t"
        "rcall bitTimeD"          "\n\t"
        // Bit 0:
        "out  %[port] , %[hi]"    "\n\t"
        "rjmp .+0"                "\n\t"
        "ld   %[byte] , %a[ptr]+" "\n\t"
        "out  %[port] , %[next]"  "\n\t"
        "mov  %[next] , %[lo]"    "\n\t"
        "sbrc %[byte] , 7"        "\n\t"
         "mov %[next] , %[hi]"    "\n\t"
        "nop"                     "\n\t"
        "out  %[port] , %[lo]"    "\n\t"
        "sbiw %[count], 1"        "\n\t"
        "brne headD"              "\n\t"
         "rjmp doneD"             "\n\t"
        "bitTimeD:"               "\n\t"
         "out  %[port], %[next]"  "\n\t"
         "mov  %[next], %[lo]"    "\n\t"
         "rol  %[byte]"           "\n\t"
         "sbrc %[byte], 7"        "\n\t"
          "mov %[next], %[hi]"    "\n\t"
         "nop"                    "\n\t"
         "out  %[port], %[lo]"    "\n\t"
         "ret"                    "\n\t"
         "doneD:"                 "\n"
        : [byte]  "+r" (b),
          [next]  "+r" (next),
          [count] "+w" (i)
        : [port]   "I" (_SFR_IO_ADDR(PORTD)),
          [ptr]    "e" (ptr),
          [hi]     "r" (hi),
          [lo]     "r" (lo));
    }
#elif (F_CPU >= 15400000UL) && (F_CPU <= 19000000L)
    volatile uint8_t next, bit;
    hi   = *port |  pin_mask;
    lo   = *port & ~pin_mask;
    next = lo;
    bit  = 8;
    asm volatile(
      "head20:"                   "\n\t"
      "st   %a[port],  %[hi]"    "\n\t"
      "sbrc %[byte],  7"         "\n\t"
       "mov  %[next], %[hi]"     "\n\t"
      "dec  %[bit]"              "\n\t"
      "st   %a[port],  %[next]"  "\n\t"
      "mov  %[next] ,  %[lo]"    "\n\t"
      "breq nextbyte20"          "\n\t"
      "rol  %[byte]"             "\n\t"
      "rjmp .+0"                 "\n\t"
      "nop"                      "\n\t"
      "st   %a[port],  %[lo]"    "\n\t"
      "nop"                      "\n\t"
      "rjmp .+0"                 "\n\t"
      "rjmp head20"              "\n\t"
     "nextbyte20:"               "\n\t"
      "ldi  %[bit]  ,  8"        "\n\t"
      "ld   %[byte] ,  %a[ptr]+" "\n\t"
      "st   %a[port], %[lo]"     "\n\t"
      "nop"                      "\n\t"
      "sbiw %[count], 1"         "\n\t"
       "brne head20"             "\n"
      : [port]  "+e" (port),
        [byte]  "+r" (b),
        [bit]   "+r" (bit),
        [next]  "+r" (next),
        [count] "+w" (i)
      : [ptr]    "e" (ptr),
        [hi]     "r" (hi),
        [lo]     "r" (lo));
#else
	#error "CPU SPEED NOT SUPPORTED"
#endif
	interrupts();
	end_time = micros();
}
