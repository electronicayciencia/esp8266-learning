/* 
    Simple library for DS1820 for ESP-IDF

    Electronicayciencia 22/01/2021
*/

#include <stdio.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"

#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h" // high resolution timer

#define TAG  "ds1820"
#define LOW  0
#define HIGH 1

#define ENTER_CRITICAL() portENTER_CRITICAL()
#define EXIT_CRITICAL() portEXIT_CRITICAL()

/* Bus line pull up recovery time (us) */
#define TREC 2 

/* Device timing parameters (see DS1820 datasheet) */
#define TRSTL 480   /* Reset Time Low (minimun time) */
#define TRSTH 480   /* Reset Time High */
#define TPDHIGH 60  /* Presence Detect High (max time) */
#define TSLOT 90    /* Time Slot (us) 60-120 */
#define TLOW1 5     /* Write 1 Low Time */
#define TLOW0 TSLOT /* Write 0 Low Time (full timeslot) */
#define TRDV 2      /* Read Data Valid */

/* Device commands (see DS1820 datasheet) */
#define CMD_SKIP_ROM 0xCC   /* Skip ROM command */
#define CMD_READ_ROM 0x33   /* Read ROM command */
#define CMD_READ_SP 0xBE    /* Read Scratchpad command */
#define CMD_CONVERT_T 0x44  /* Convert T command */

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
int ds1820_reset (gpio_num_t pin) {
    int v;

    ENTER_CRITICAL();
    low(pin, TRSTL);           /* Reset pulse */
    ets_delay_us(TPDHIGH);     /* Wait 15-60 and answer back*/
    v = gpio_get_level(pin);   /* DS1820 pulls down if present */
    EXIT_CRITICAL();
    ets_delay_us(TRSTH-TPDHIGH+TREC);
    return !v;
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

/* Reads ROM, void function just for testing */
void ds1820_read_rom(gpio_num_t pin) {
    ESP_LOGI(TAG, "Reading ROM data (Cmd 33h)");
    send_byte(pin, CMD_READ_ROM);
    char rom_data[8];

    int i;
    for (i = 0; i < 8; i++) {
        rom_data[i] = read_byte(pin);
    }

    ESP_LOGI(TAG, "ROM data: %02X %02X %02X %02X %02X %02X %02X %02X - CRC %s",
        rom_data[0],
        rom_data[1],
        rom_data[2],
        rom_data[3],
        rom_data[4],
        rom_data[5],
        rom_data[6],
        rom_data[7],
        crc8(rom_data, 8) ? "error" : "OK");

    ds1820_reset(pin);
}

/* Starts a temperature convertion and waits for it to be done */
void convert_t (gpio_num_t pin) {
    send_byte(pin, CMD_SKIP_ROM);
    send_byte(pin, CMD_CONVERT_T);

    while (read_byte(pin) != 0xFF)
        vTaskDelay(20 / portTICK_RATE_MS);  // up to 500ms

    ds1820_reset(pin);
}

void read_scratchpad(gpio_num_t pin, char *buff) {
    send_byte(pin, CMD_SKIP_ROM);
    send_byte(pin, CMD_READ_SP);
    int i;
    for (i = 0; i < 9; i++) {
        buff[i] = read_byte(pin);
    }
    
    ESP_LOGD(TAG, "Scratchpad: %02X %02X %02X %02X %02X %02X %02X %02X %02X - CRC %s",
        buff[0],
        buff[1],
        buff[2],
        buff[3],
        buff[4],
        buff[5],
        buff[6],
        buff[7],
        buff[8],
        crc8(buff, 9) ? "error" : "OK");

    ds1820_reset(pin);
}

/* Not tested with negative temperature */
float ds1820_read_temp(gpio_num_t pin) {
    convert_t(pin);

    char scratchpad[9];
    read_scratchpad(pin, scratchpad);

    int8_t  temp_read    = scratchpad[0];
    uint8_t count_remain = scratchpad[6];
    uint8_t count_per_c  = scratchpad[7];

    /* Low res temp */
    //return (float) temp_read / 2.0;

    /* High res temp */
    float temp_hr = (int) temp_read / 2;
    temp_hr = temp_hr - 0.25 + ((float)count_per_c - (float)count_remain) / (float)count_per_c;
    return temp_hr;
}
