

#ifndef __RESPEAKER_H__
#define __RESPEAKER_H__

#include "Arduino.h"

#define SPI_BUF_SIZE            64
#define TOUCH_NUM               8
#define TOUCH_DETECT_INTERVAL   50 // ms
#define TOUCH_DEFAULT_THRESHOLD 20
#define PIXELS_NUM              12

class ReSpeaker
{
public:
    ReSpeaker();
    
    void begin(int touch=1, int pixels=1, int spi=1);
    
    void play(const char *name);
    
    void exec(const char *cmd);
    
    uint8_t detect_touch();
    
    void handle_spi_data(uint8_t data);
    
    void attach_touch_isr(void (*isr)(uint8_t id, uint8_t event)) {
        touch_isr = isr;
    }
    
    void attach_spi_isr(void (*isr)(uint8_t addr, uint8_t *data, uint8_t len)) {
        spi_isr = isr;
    }
    
    void attach_spi_isr_raw(void (*isr)(uint8_t data)) {
        spi_isr_raw = isr;
    }
    
public:
    void (*spi_isr_raw)(uint8_t data);
    void (*touch_isr)(uint8_t id, uint8_t event);
    uint32_t last_touch_detected;
    
private:
    uint8_t spi_buf_index;
    uint8_t *spi_buf;
    static const uint8_t touch_pins[TOUCH_NUM];
    uint8_t *touch_data;
    uint16_t touch_threshold;
    
    void (*spi_isr)(uint8_t addr, uint8_t *data, uint8_t len);
};

extern ReSpeaker respeaker;

#endif // __RESPEAKER_H__
