
#include "respeaker.h"
#include "SPI.h"


ReSpeaker respeaker;

// disorganize touch pins order to reduce interference {8, 9, 13, 10, 6, 2, 4, 3}
const uint8_t ReSpeaker::touch_pins[TOUCH_NUM] = {9, 2, 10, 3, 8, 6, 13, 4};
const uint8_t ReSpeaker::touch_pins_id[TOUCH_NUM] = {1, 5, 3, 7, 0, 4, 2, 6};

ReSpeaker::ReSpeaker()
{
    console = 1;
    
    touch_handler = 0;
    last_touch_detected = 0;
    touch_threshold = TOUCH_DEFAULT_THRESHOLD;
    
    spi_raw_handler = 0;
    spi_handler = 0;
    spi_state = 0;
    spi_buf_index = 0;
}
    
void ReSpeaker::begin(int touch, int pixels, int spi)
{
    Serial.begin(57600);
    Serial1.begin(57600);
    
    if (touch) {
        touch_data = new uint8_t[TOUCH_NUM];
        
        for (uint8_t i = 0; i < TOUCH_NUM; i++) {
            pinMode(touch_pins[i], OUTPUT);
            digitalWrite(touch_pins[i], LOW);
        }
    }

    if (pixels) {
        pixels_ptr = new Pixels(PIXELS_NUM);
        pixels_ptr->begin(PIXELS_PIN);
        pixels_ptr->update();
    }
    
    if (spi) {
        spi_buf = new uint8_t[SPI_BUF_SIZE];
        
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

uint8_t crc8(const uint8_t *data, uint8_t len)
{
    uint16_t crc = 0x00;
    while (len--) {
        crc ^= (*data++ << 8);
        for(uint8_t i = 8; i; i--) {
            if (crc & 0x8000) {
                crc ^= (0x1070 << 3);
            }
            crc <<= 1;
        }
    }
    return (uint8_t)(crc >> 8);
}

void ReSpeaker::handle_spi_data(uint8_t data)
{
    if (0 == spi_state) {
        if (SPI_DATA_PREFIX == data) {
            spi_state = 1;
        }
    } else if (1 == spi_state) {
        spi_data_address = data;
        spi_state = 2;
    } else if (2 == spi_state) {
        if (data <= SPI_BUF_SIZE) {
            spi_data_length = data;
            spi_buf_index = 0;
            spi_state = 3;
        } else {
            spi_state = 0;
        }
    } else if (3 == spi_state) {
        spi_buf[spi_buf_index] = data;
        spi_buf_index++;
         
        if (spi_data_length <= spi_buf_index) {
            spi_state = 4;
        }
    } else if (4 == spi_state) {
        uint8_t crc = crc8(spi_buf, spi_data_length);
        if (crc == data) {
            spi_handler(spi_data_address, spi_buf, spi_data_length);
        } else {
            Serial.print("crc check failed:");
            Serial.println(crc);
            for (uint8_t i = 0; i < spi_data_length; i++) {
                Serial.print(spi_buf[i]);
                Serial.print(' ');
            }
            Serial.println(data);
        }
        spi_state = 0;
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
            if (touch_handler) touch_handler(touch_pins_id[i], 1);
        } else if (0x80 == touch_data[i]) {
            status &= ~(1 << i);
            if (touch_handler) touch_handler(touch_pins_id[i], 0);
        }
    }
    
    return status;
}

uint16_t ReSpeaker::read_touch(uint8_t id)
{
    uint16_t count;
    uint8_t index = 0;
    for (uint8_t i=0; i<TOUCH_NUM; i++) {
        if (id == touch_pins_id[i]) {
            index = i;
            break;
        }
    }

    pinMode(touch_pins[index], INPUT);
    while (!digitalRead(touch_pins[index])) {
        count++;
    }

    pinMode(touch_pins[index], OUTPUT);
    digitalWrite(touch_pins[index], LOW);
    
    return count;
}

void ReSpeaker::_loop()
{
    if (console) {
        while (Serial.available() && Serial1.availableForWrite()) {
            Serial1.write((char)Serial.read());
        }

        while (Serial1.available() && Serial.availableForWrite()) {
            Serial.write((char)Serial1.read());
        }
    }
    
    if (touch_handler) {
        uint32_t current = millis();
        if ((uint32_t)(current - last_touch_detected) >= 50) {
            last_touch_detected = current;
            
            detect_touch();
        }
    }
}

void serialEventRun()
{
    respeaker._loop();
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
