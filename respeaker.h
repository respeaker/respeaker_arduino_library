

#ifndef __RESPEAKER_H__
#define __RESPEAKER_H__

#include "Arduino.h"

#define SPI_BUF_SIZE            64
#define TOUCH_NUM               8
#define TOUCH_DETECT_INTERVAL   50 // ms
#define TOUCH_DEFAULT_THRESHOLD 32
#define PIXELS_NUM              12

class ReSpeaker
{
public:
    ReSpeaker() {
        console = 1;
    }
    
    /**
     * setup touch buttons, full color pixels and spi bridge
     *
     * @param touch     1 - enable touch buttons, 0 - disabled
     * @param pixels    1 - enable pixels, 0 - disabled
     * @param spi       1 - enable spi bridge, 0 - disabled
     */
    void begin(int touch=1, int pixels=1, int spi=1);
    
    /**
     * play music file on sd card, support wav & mp3
     *
     * @param name      music file without path
     */
    void play(const char *name);
    
    /**
     * execute a shell command
     *
     * @param cmd      command to be executed
     */
    void exec(const char *cmd);
    
    /**
     *
     *
     */
    void handle_spi_data(uint8_t data);
    
    /**
     * adjust touch threshold
     *
     * @return threshold
     */
    uint16_t calibrate_touch();

    /**
     * charge a touch button's capacity, read the charging time
     *
     * @return measurement value
     */
    uint16_t read_touch(uint8_t id);
    
    /**
     * detect all touch buttons
     *
     * return touch buttons' status
     */
    uint16_t detect_touch();
    
    /**
     * set touch threshold
     *
     * @param threshold
     */
    void set_touch_threshold(uint16_t threshold) {
        touch_threshold = threshold;
    }
    
    /**
     * enable/disable usb to serial bridge
     *
     * @param enable    1 or 0
     */
    void usb_to_serial(uint8_t enable=1) {
        console = enable;
    }
    
    /**
     * attach an interrupt handler which will be called when a touch event happens
     *
     * @param isr   interrupt handler
     */
    void attach_touch_isr(void (*isr)(uint8_t id, uint8_t event)) {
        touch_isr = isr;
    }
    
    /**
     * attach an interrupt handler which will be called when a spi packet is received
     *
     * @param isr   interrupt handler
     */
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
    uint8_t console;
    
private:
    uint8_t spi_buf_index;
    uint8_t *spi_buf;
    uint8_t *touch_data;
    uint16_t touch_threshold;
    static const uint8_t touch_pins[TOUCH_NUM];
    
    void (*spi_isr)(uint8_t addr, uint8_t *data, uint8_t len);
};

extern ReSpeaker respeaker;

#endif // __RESPEAKER_H__
