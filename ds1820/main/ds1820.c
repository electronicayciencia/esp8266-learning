/* 
    Simple library for DS1820 for ESP-IDF

    For devices in the same port this library is not thread-safe.

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
#define TCONV 94    /* Temperature Conversion Time (for 9 bits) milliseconds */

/* Device commands (see DS1820 datasheet) */
typedef enum {
    CMD_SKIP_ROM  = 0xCC,  /* Skip ROM command */
    CMD_READ_PWR  = 0xB4,  /* Read Power Supply command */
    CMD_READ_ROM  = 0x33,  /* Read ROM command */
    CMD_READ_SP   = 0xBE,  /* Read Scratchpad command */
    CMD_CONVERT_T = 0x44,  /* Convert T command */
    CMD_MATCH_ROM = 0x55   /* Match ROM command */
} ds1820_cmd_t;



/* Pulls down the bus for given us and then releases it */
static void low(gpio_num_t pin, uint32_t us) {
    gpio_set_level(pin, LOW);
    ets_delay_us(us);
    gpio_set_level(pin, HIGH);
}

/* Sends a 0 by pulling the bus for a whole time slot
 * Sends a 1 by pulling just a bit (10-15ms),
 * and then releasing it for the rest of time slot */
static void send_bit(gpio_num_t pin, bool bit) {
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
static void send_byte(ds1820_device_t *dev, char cmd) {
    int i;

    for (i = 0; i < 8; i++) {
        send_bit(dev->pin, cmd & 1);
        cmd = cmd >> 1;
    }
}

/* Sends a brief pulse and then read for the response */
static bool read_bit(gpio_num_t pin) {
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
static char read_byte(gpio_num_t pin) {
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

static void strong_pullup(ds1820_device_t *dev, uint32_t ms) {
    gpio_set_direction(dev->pin, GPIO_MODE_OUTPUT);
    gpio_set_level(dev->pin, HIGH);

    ESP_LOGD(TAG, "Strong pullup for %d ms", ms);

    vTaskDelay(ms / portTICK_RATE_MS);
    
    gpio_set_direction(dev->pin, GPIO_MODE_OUTPUT_OD);
}

/* Reset the bus where the device is and wait for a presence response */
static ds1820_err_t reset (ds1820_device_t *dev) {
    ENTER_CRITICAL();
    low(dev->pin, TRSTL);               /* Reset pulse */
    ets_delay_us(TPDHIGH);              /* Wait 15-60us */
    int v = gpio_get_level(dev->pin);   /* DS1820 pulls down if present */
    EXIT_CRITICAL();
    ets_delay_us(TRSTH-TPDHIGH+TREC);
    
    if (v == HIGH) {
        return DS1820_ERR_EMPTYBUS;
    }

    return DS1820_ERR_OK;
}

/* Calculates CRC8-Maxim according datasheet
 * If CRC is appended at the end of string, correct array gives result 00 */
static unsigned char crc8 (char *str, size_t len) {
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
static ds1820_err_t read_rom(ds1820_device_t *dev) {
    ESP_LOGI(TAG, "Reading ROM data (Cmd 33h)");
    send_byte(dev, CMD_READ_ROM);

    int i;
    for (i = 0; i < 8; i++) {
        dev->rom[i] = read_byte(dev->pin);
    }

    int remainder = crc8(dev->rom, 8);

    ESP_LOGI(TAG, "ROM data: %02X %02X %02X %02X %02X %02X %02X %02X - CRC %s",
        dev->rom[0],
        dev->rom[1],
        dev->rom[2],
        dev->rom[3],
        dev->rom[4],
        dev->rom[5],
        dev->rom[6],
        dev->rom[7],
        remainder ? "error" : "OK");

    if (reset(dev) != DS1820_ERR_OK) {
        return DS1820_ERR_EMPTYBUS;
    }

    if (remainder != 0) {
        return DS1820_ERR_BADCRC;
    }

    return DS1820_ERR_OK;
}

/* Call the device in dev. 
   If the ROM is NULL pointer, it uses SKIP ROM instead. */
static void address_device(ds1820_device_t *dev) {
    if (dev->rom == NULL) {
        send_byte(dev, CMD_SKIP_ROM);
    }
    else {
        send_byte(dev, CMD_MATCH_ROM);
        for (int i = 0; i < 8; i++) {
            send_byte(dev, dev->rom[i]);
        }
    }
}


/* Read power mode used by the devices. */
static ds1820_pwr_t read_power_mode(ds1820_device_t *dev) {
    address_device(dev);
    send_byte(dev, CMD_READ_PWR);
    int v = read_bit(dev->pin);

    if (reset(dev) != DS1820_ERR_OK) {
        return DS1820_ERR_EMPTYBUS;
    }

    return v == LOW ? DS1820_PWR_PARASITE
                    : DS1820_PWR_VCC;
}

/* Starts a temperature conversion and waits for it to be done */
static ds1820_err_t convert_t (ds1820_device_t *dev) {
    address_device(dev);
    send_byte(dev, CMD_CONVERT_T);

    if (dev->power == DS1820_PWR_PARASITE) {
        strong_pullup(dev, TCONV << (dev->resolution_bits - 9));
    }
    else {
        while (read_byte(dev->pin) != 0xFF)
            vTaskDelay(20 / portTICK_RATE_MS);  // ask every 20 ms
    }
    
    if (reset(dev) != DS1820_ERR_OK) {
        return DS1820_ERR_EMPTYBUS;
    }

    return DS1820_ERR_OK;
}

/* Return DS1820_ERR_OK if OK and DS1820_ERR_BADCRC if error */
ds1820_err_t read_scratchpad(ds1820_device_t *dev) {
    int allones = 0;

    address_device(dev);
    send_byte(dev, CMD_READ_SP);
    int i;
    for (i = 0; i < 9; i++) {
        dev->scratchpad[i] = read_byte(dev->pin);
        if (dev->scratchpad[i] == 0xFF) {
            allones++; // device is not there?
        }
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
        crc_remainder == 0 ? "OK" : "error");

    if (reset(dev) != DS1820_ERR_OK) {
        return DS1820_ERR_EMPTYBUS;
    }

    // Device definitely is not there
    if (allones >= 9) {
        return DS1820_ERR_NOANSWER;
    }

    return crc_remainder == 0 ? DS1820_ERR_OK
                              : DS1820_ERR_BADCRC;
}

/* Not tested with negative temperature */
/* Return DS1820_ERR_OK if OK and DS1820_ERR_BADCRC if error */
ds1820_err_t ds1820_read_temp(ds1820_device_t *dev, float *temp) {
    ds1820_err_t result;

    result = convert_t(dev);
    if (result != DS1820_ERR_OK) return result;

    result = read_scratchpad(dev);
    if (result != DS1820_ERR_OK) return result;

    /* DS1820 Low res temp (fixed point arithmertic) */
    //temp = (float) temp_read / 2;
    /* DS1820 High res temp (floating point arithmetic) */
    if (dev->family == DS1820_FAMILY_DS18S20) {
        int8_t  temp_read    = dev->scratchpad[0];
        uint8_t count_remain = dev->scratchpad[6];
        uint8_t count_per_c  = dev->scratchpad[7];

        float temp_hr = (int) temp_read / 2;
        temp_hr = temp_hr - 0.25 + ((float)count_per_c - (float)count_remain) / (float)count_per_c;
        *temp = temp_hr;
    }
    /* DS18B20 12 bit resolution */
    else {
        *temp = ((dev->scratchpad[1] << 8) + dev->scratchpad[0]) / 16.0;
    }

    result = reset(dev);
    if (result != DS1820_ERR_OK) return result;

    return DS1820_ERR_OK;
}


/* Initialization. Read power supply and reset */
/* Return null on error */
ds1820_device_t *ds1820_init(gpio_num_t pin, const char *rom) {

    ds1820_device_t *dev = malloc(sizeof(ds1820_device_t));

    if (dev == NULL) {
        ESP_LOGW(TAG, "Cannot allocate memory.");
        return NULL;
    }
    
    dev->pin = pin;
    dev->resolution_bits = 12; // only default supported

    /* configure pin open drain (weak pullup) */
    gpio_set_direction(dev->pin, GPIO_MODE_OUTPUT_OD);
    gpio_pullup_en(dev->pin);

    /* Check if any device exists */
    if (reset(dev) != DS1820_ERR_OK) {
        ESP_LOGW(TAG, "No devices in 1-wire bus.");
        return NULL;
    }

    ESP_LOGI(TAG, "At least 1 device detected!");

    /* Learn rom. Only if one device in bus. */
    if (rom != NULL) {
        memcpy(dev->rom, rom, 8);
    }
    else {
        if (read_rom(dev) != DS1820_ERR_OK) {
            ESP_LOGW(TAG, "Error reading ROM. Multiple devices?");
            return NULL;
        }
    }

    /* Learn variant */
    switch (dev->rom[0]) {
        case DS1820_FAMILY_DS18S20:
            dev->family = DS1820_FAMILY_DS18S20;
            ESP_LOGI(TAG, "Device family is DS1820 or DS18S20.");
            break;

        case DS1820_FAMILY_DS18B20:
            dev->family = DS1820_FAMILY_DS18B20;
            ESP_LOGI(TAG, "Device family is DS18B20.");
            break;

        default:
            ESP_LOGW(TAG, "Unkown device family: %02X.", dev->rom[0]);
            return NULL;
    }

    /* Learn power mode */
    if (read_power_mode(dev) == DS1820_PWR_PARASITE) {
        ESP_LOGI(TAG, "One o more devices are using parasitic power. "
                      "Strong pull up active.");
        dev->power = DS1820_PWR_PARASITE;
    }

    return dev;
}

/* TODO: free */
