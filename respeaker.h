

#ifndef __RESPEAKER_H__
#define __RESPEAKER_H__

#include "Arduino.h"
#include "pixels.h"

#define SPI_BUF_SIZE            64
#define SPI_DATA_PREFIX		0xA5
#define TOUCH_NUM               8
#define TOUCH_DETECT_INTERVAL   4 // ms
#define TOUCH_DEFAULT_THRESHOLD 32
#define PIXELS_NUM              12
#define PIXELS_PIN              11

class ReSpeaker
{
public:
    ReSpeaker();
    
    /**
     * setup touch buttons, full color pixels and spi bridge
     *
     * @param touch     1 - enable touch buttons, 0 - disable
     * @param pixels    1 - enable pixels, 0 - disable
     * @param spi       1 - enable spi bridge, 0 - disable
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
     * @param enable    0 - disable console, otherwise enable console
     */
    void set_console(uint8_t enable=1) {
        console = enable;
    }
    
    /**
     * attach an interrupt handler which will be called when a touch event happens
     *
     * @param handler   touch interrupt handler
     */
    void attach_touch_handler(void (*handler)(uint8_t id, uint8_t event)) {
        touch_handler = handler;
    }
    
    /**
     * attach an interrupt handler which will be called when a spi packet is received
     *
     * @param handler   spi interrupt handler
     */
    void attach_spi_handler(void (*handler)(uint8_t addr, uint8_t *data, uint8_t len)) {
        spi_handler = handler;
    }
    
        
    /**
     * receive spi raw data, convert raw data into a packet with address and lenght
     *
     * @param data      spi received raw data
     */
    void handle_spi_data(uint8_t data);
    
    /**
     * attach an interrupt handler which will be called when a single byte is received from spi
     *
     * @param handler   raw spi interrupt handler
     */
    void attach_spi_raw_handler(void (*handler)(uint8_t data)) {
        spi_raw_handler = handler;
    }

    /**
     * Get the Pixels reference of the 12 pixels on respeaker
     * 
     * @return  Pixels reference 
     */
    Pixels &pixels() {
        return *pixels_ptr;
    }
    
    /**
     * called by serialEventRun() repeatedly
     */
    void _loop();
    
public:
    void (*spi_raw_handler)(uint8_t data);
    void (*spi_handler)(uint8_t addr, uint8_t *data, uint8_t len);
    
private:
    Pixels *pixels_ptr;
    uint8_t *touch_data;
    
    static const uint8_t touch_pins[TOUCH_NUM];
    static const uint8_t touch_pins_id[TOUCH_NUM];
    uint16_t touch_threshold;
    void (*touch_handler)(uint8_t id, uint8_t event);
    uint32_t last_touch_detected;
    
    uint8_t spi_state;
    uint8_t spi_data_address;
    uint8_t spi_data_length;
    uint8_t spi_buf_index;
    uint8_t *spi_buf;
    
    uint8_t console;
};

extern ReSpeaker respeaker;

#endif // __RESPEAKER_H__
