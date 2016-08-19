
#include "respeaker.h"
#include "SPI.h"


ReSpeaker respeaker;
const uint8_t ReSpeaker::touch_pins[TOUCH_NUM] = {6, 2, 4, 3, 8, 9, 13, 10};

    
void ReSpeaker::begin(int touch, int pixels, int spi)
{
    Serial.begin(57600);
    Serial1.begin(57600);
    
    if (touch) {
        touch_data = new uint8_t[TOUCH_NUM];
        last_touch_detected = 0;
        touch_handler = 0;
        touch_threshold = TOUCH_DEFAULT_THRESHOLD;
        
        for (uint8_t i = 0; i < TOUCH_NUM; i++) {
            pinMode(touch_pins[i], OUTPUT);
            digitalWrite(touch_pins[i], LOW);
        }
    } else {
        touch_data = 0;
    }

    if (pixels) {
        pixels_ptr = new Pixels(PIXELS_NUM);
        pixels_ptr->begin(PIXELS_PIN);
    }
    
    if (spi) {
        spi_buf = new uint8_t[SPI_BUF_SIZE];
        spi_buf_index = 0;
        spi_raw_handler = 0;
        
        pinMode(MOSI, INPUT);
        pinMode(MISO, OUTPUT);
        pinMode(SCK, INPUT);
        pinMode(SS, INPUT);

        // use SPI slave mode
        SPCR |= _BV(SPE);

        SPI.attachInterrupt();
    }
}

void ReSpeaker::play(const char *name)
{
    Serial1.print("play ");
    Serial1.print(name);
    Serial1.print('\n');
}

void ReSpeaker::exec(const char *cmd)
{
    Serial1.print(cmd);
    Serial1.print('\n');
}

void ReSpeaker::handle_spi_data(uint8_t data)
{
    if (spi_buf_index >= SPI_BUF_SIZE) {
        spi_buf_index = 0;
    }
    
    if ('\n' == data) {
        spi_buf[spi_buf_index] = '\0';
        if (spi_handler) spi_handler(0, spi_buf, spi_buf_index);
        spi_buf_index = 0;
    } else {
        spi_buf[spi_buf_index] = data;
        spi_buf_index++;
    }
}

uint16_t ReSpeaker::detect_touch()
{
    uint16_t status;
    for (uint8_t i = 0; i < TOUCH_NUM; i++) {
        uint8_t count = 0;
        touch_data[i] <<= 1;
        pinMode(touch_pins[i], INPUT);
        while (!digitalRead(touch_pins[i])) {
            count++;
            if (count >= touch_threshold) {
                touch_data[i] |= 0x01;
                break;
            }
        }

        pinMode(touch_pins[i], OUTPUT);
        digitalWrite(touch_pins[i], LOW);

        if (0x01 == touch_data[i]) {
            status |= 1 << i;
            if (touch_handler) touch_handler(i, 1);
        } else if (0x80 == touch_data[i]) {
            status &= ~(1 << i);
            if (touch_handler) touch_handler(i, 0);
        }
    }
    
    return status;
}

uint16_t ReSpeaker::read_touch(uint8_t id)
{
    uint16_t count;

    pinMode(touch_pins[id], INPUT);
    while (!digitalRead(touch_pins[id])) {
        count++;
    }

    pinMode(touch_pins[id], OUTPUT);
    digitalWrite(touch_pins[id], LOW);
    
    return count;
}

void serialEventRun()
{
    if (respeaker.console) {
        while (Serial.available() && Serial1.availableForWrite()) {
            Serial1.write((char)Serial.read());
        }

        while (Serial1.available() && Serial1.availableForWrite()) {
            Serial.write((char)Serial1.read());
        }
    }
    
    if (respeaker.touch_handler) {
        uint32_t current = millis();
        if ((uint32_t)(current - respeaker.last_touch_detected) >= 50) {
            respeaker.last_touch_detected = current;
            
            respeaker.detect_touch();
        }
    }
}

// SPI Interrupt Service Routine
ISR (SPI_STC_vect)
{
    uint8_t data = SPDR;  // read SPI Data Register

    if (!respeaker.spi_raw_handler) {
        respeaker.handle_spi_data(data);
    } else {
        respeaker.spi_raw_handler(data);
    }
}
