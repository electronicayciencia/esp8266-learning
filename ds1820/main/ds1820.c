/* 
    Simple library for DS1820 for ESP-IDF

    Electronicayciencia 22/01/2021
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"

#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h" // high resolution timer

#include "ds1820.h"

#define TAG  "ds1820"
#define LOW  0
#define HIGH 1

#define ENTER_CRITICAL() portENTER_CRITICAL()
#define EXIT_CRITICAL() portEXIT_CRITICAL()

/* Bus line pull up recovery time (us) */
#define TREC 2 

/* Device timing parameters (see DS1820 datasheet) */
#define TRSTL 480   /* Reset Time Low (minimum time) */
#define TRSTH 480   /* Reset Time High */
#define TPDHIGH 60  /* Presence Detect High (max time) */
#define TSLOT 90    /* Time Slot (us) 60-120 */
#define TLOW1 5     /* Write 1 Low Time */
#define TLOW0 TSLOT /* Write 0 Low Time (full timeslot) */
#define TRDV 2      /* Read Data Valid */

/* Device commands (see DS1820 datasheet) */
#define CMD_SKIP_ROM  0xCC  /* Skip ROM command */
#define CMD_READ_PWR  0xB4  /* Read Power Supply command */
#define CMD_READ_ROM  0x33  /* Read ROM command */
#define CMD_READ_SP   0xBE  /* Read Scratchpad command */
#define CMD_CONVERT_T 0x44  /* Convert T command */

/* Power supply model */
#define DS1820_PWR_VCC 0
#define DS1820_PWR_PARASITE 1

/* Family */
#define DS1820_FAMILY_DS18S20 0x10
#define DS1820_FAMILY_DS18B20 0x28
#define DS1820_FAMILY_DS1825  0x3B


/* Pulls down the bus for given us and then releases it */
void low(gpio_num_t pin, int us) {
    gpio_set_level(pin, LOW);
    ets_delay_us(us);
    gpio_set_level(pin, HIGH);
}

/* Sends a 0 by pulling the bus for a whole time slot
 * Sends a 1 by pulling just a bit (10-15ms),
 * and then releasing it for the rest of time slot */
void send_bit(gpio_num_t pin, int bit) {
    ENTER_CRITICAL();
    if (bit == HIGH) {
        low(pin, TLOW1);
        ets_delay_us(TSLOT-TLOW1);
    }
    else {
        low(pin, TLOW0);
    }
    ets_delay_us(TREC);
    EXIT_CRITICAL();
}

/* Sends 8 bit in a row, LSB first */
void send_byte(gpio_num_t pin, char byte) {
    int i;

    for (i = 0; i < 8; i++) {
        send_bit(pin, byte & 1);
        byte = byte >> 1;
    }
}

/* Sends a brief pulse and then read for the response */
int read_bit(gpio_num_t pin) {
    int s;

    ENTER_CRITICAL();
    low(pin, TRDV);
    s = gpio_get_level(pin);
    ets_delay_us(TSLOT);
    EXIT_CRITICAL();

    //printf("%s", s ? "1" : "0");
    return s;
}

/* Reads a byte, LSB first */
char read_byte(gpio_num_t pin) {
    int byte = 0x00;
    int i;

    for (i=0; i<8; i++) {
        int b;
        b = read_bit(pin);
        b = b << i;
        byte = byte | b;
        ets_delay_us(TREC);
    }

    return byte;
}

/* Sends a reset pulse and waits for a presence response */
ds1820_err_t ds1820_reset (ds1820_device_t *dev) {
    int v;

    ENTER_CRITICAL();
    low(dev->pin, TRSTL);           /* Reset pulse */
    ets_delay_us(TPDHIGH);     /* Wait 15-60 and answer back*/
    v = gpio_get_level(dev->pin);   /* DS1820 pulls down if present */
    EXIT_CRITICAL();
    ets_delay_us(TRSTH-TPDHIGH+TREC);
    
    if (v == HIGH) {
        return DS1820_ERR_NODEVICE;
    }

    return DS1820_ERR_OK;
}

/* Calculates CRC8-Maxim according datasheet
 * If CRC is appended at the end of string, correct array gives result 00 */
unsigned char crc8 (char *str, size_t len) {
    char div = 0b10001100; // Rotated poly
    unsigned char crc = 0;

    size_t i;
    for (i = 0; i < len; i++) {
        unsigned char byte = str[i];

        int j;
        for (j = 0; j < 8; j++) {

            // Shift CRC
            char crc_carry = crc & 1;
            crc >>= 1;

            // Shift Byte
            char byte_carry = byte & 1;
            byte >>= 1;

            // If crc_carry XOR byte_carry we make crc XOR div
            if (crc_carry ^ byte_carry)
                crc ^= div;
        }

    }

    return crc;
}

/* Reads device ROM and fills ROM field.
   Only usable when there is just one device in the bus. */
ds1820_err_t ds1820_read_rom(ds1820_device_t *dev) {
    ESP_LOGI(TAG, "Reading ROM data (Cmd 33h)");
    send_byte(dev->pin, CMD_READ_ROM);

    int i;
    for (i = 0; i < 8; i++) {
        dev->rom[i] = read_byte(dev->pin);
    }

    ESP_LOGI(TAG, "ROM data: %02X %02X %02X %02X %02X %02X %02X %02X - CRC %s",
        dev->rom[0],
        dev->rom[1],
        dev->rom[2],
        dev->rom[3],
        dev->rom[4],
        dev->rom[5],
        dev->rom[6],
        dev->rom[7],
        crc8(dev->rom, 8) ? "error" : "OK");

    if (ds1820_reset(dev->pin) != DS1820_ERR_OK) {
        return DS1820_ERR_NODEVICE;
    }

    return DS1820_ERR_OK;
}

/* Starts a temperature convertion and waits for it to be done */
ds1820_err_t convert_t (ds1820_device_t *dev) {
    send_byte(dev->pin, CMD_CONVERT_T);

    while (read_byte(dev->pin) != 0xFF)
        vTaskDelay(20 / portTICK_RATE_MS);  // up to 500ms
    
    return DS1820_ERR_OK;
}

/* Return DS1820_ERR_OK if OK and DS1820_ERR_BADCRC if error */
ds1820_err_t read_scratchpad(ds1820_device_t *dev) {

    if (ds1820_reset(dev->pin) != DS1820_ERR_OK) {
        return DS1820_ERR_NODEVICE;
    }

    send_byte(dev->pin, CMD_SKIP_ROM);
    send_byte(dev->pin, CMD_READ_SP);
    int i;
    for (i = 0; i < 9; i++) {
        dev->scratchpad[i] = read_byte(dev->pin);
    }
    
    int crc_remainder = crc8(dev->scratchpad, 9);

    ESP_LOGD(TAG, "Scratchpad: %02X %02X %02X %02X %02X %02X %02X %02X %02X - CRC %s",
        dev->scratchpad[0],
        dev->scratchpad[1],
        dev->scratchpad[2],
        dev->scratchpad[3],
        dev->scratchpad[4],
        dev->scratchpad[5],
        dev->scratchpad[6],
        dev->scratchpad[7],
        dev->scratchpad[8],
        crc_remainder ? "error" : "OK");

    if (ds1820_reset(dev) != DS1820_ERR_OK) {
        return DS1820_ERR_NODEVICE;
    }

    if (crc_remainder) {
        return DS1820_ERR_BADCRC;
    }
    else {
        return DS1820_ERR_OK;
    }
}

/* Not tested with negative temperature */
/* Return DS1820_ERR_OK if OK and DS1820_ERR_BADCRC if error */
ds1820_err_t ds1820_read_temp(ds1820_device_t *dev, float *temp) {
    if (ds1820_reset(dev->pin) != DS1820_ERR_OK) {
        return DS1820_ERR_NODEVICE;
    }

    send_byte(dev, CMD_SKIP_ROM);
    convert_t(dev);

    if (ds1820_reset(dev) != DS1820_ERR_OK) {
        return DS1820_ERR_NODEVICE;
    }

    char scratchpad[9];
    int result = read_scratchpad(dev);

    int8_t  temp_read    = dev->scratchpad[0];
    uint8_t count_remain = dev->scratchpad[6];
    uint8_t count_per_c  = dev->scratchpad[7];

    /* Low res temp (fixed point arithmertic) */
    //temp = (float) temp_read / 2;
    /* High res temp (floating point arithmetic) */
    float temp_hr = (int) temp_read / 2;
    temp_hr = temp_hr - 0.25 + ((float)count_per_c - (float)count_remain) / (float)count_per_c;
    *temp = temp_hr;
    // For 18b20
    // *temp = ((scratchpad[1] << 8) + scratchpad[0]) / 16.0;

    if (ds1820_reset(dev) != DS1820_ERR_OK) {
        return DS1820_ERR_NODEVICE;
    }

    if (result == DS1820_ERR_BADCRC) {
        return DS1820_ERR_BADCRC;
    }
    else {
        return DS1820_ERR_OK;
    }
}


/* Initialization. Read power supply and reset */
/* Return null on error */
ds1820_device_t *ds1820_init(gpio_num_t pin, const char *rom) {
    ds1820_device_t *dev = malloc(sizeof(ds1820_device_t));

    if (dev == NULL) {
        ESP_LOGE(TAG, "Cannot allocate memory.");
        return NULL;
    }
    dev->pin = pin;
    dev->model = model;


    /* configure pin open drain (weak pullup) */
    gpio_set_direction(dev->pin, GPIO_MODE_OUTPUT_OD);
    gpio_pullup_en(dev->pin);

    /* Check if any device exists */
    if (ds1820_reset(dev) != DS1820_ERR_OK) {
        ESP_LOGE(TAG, "No devices in 1-wire bus.");
        return NULL;
    }

    /* Learn rom */
    if (rom) {
        ESP_LOGW(TAG, "Multiple devices in same bus not supported yet.");
        memcpy(dev->rom, rom, 8);
        /* TODO: Check if that device actually answer */
    }
    else {
        if (ds1820_read_rom(dev) != DS1820_ERR_OK) {
            ESP_LOGE(TAG, "Error reading ROM. Multiple devices?");
            return NULL;
        }
    }

    /* Learn variant */
    switch (dev->rom[0]) {
        case DS1820_FAMILY_DS18S20:
            dev->family = DS1820_FAMILY_DS18S20;
            ESP_LOGI(TAG, "Device is a DS1820 or DS18S20.");
            break;

        case DS1820_FAMILY_DS18B20:
            dev->family = DS1820_FAMILY_DS18B20;
            ESP_LOGI(TAG, "Device is a DS18B20.");
            break;

        case DS1820_FAMILY_DS1825:
            ESP_LOGE(TAG, "Device DS1825 is not supported yet.");
            return NULL;

        default:
            ESP_LOGE(TAG, "Unkown device family: %02X.", dev->rom[0]);
            return NULL;
    }

    /* Learn power mode */

}

/* todo: free */
