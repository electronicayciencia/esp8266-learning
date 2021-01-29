/* 
    Simple library for DS1820 for ESP-IDF

    Electronicayciencia 22/01/2021
*/
#ifndef DS1820_H
#define DS1820_H

#define DS1820_ROM_UNKNOWN NULL
#include "driver/gpio.h"

/* Errors */
typedef enum {
    DS1820_ERR_OK,       // success
    DS1820_ERR_BADCRC,   // invalid crc
    DS1820_ERR_NOANSWER, // selected device did not respond
    DS1820_ERR_EMPTYBUS  // no devices present (presence pulse)
} ds1820_err_t;

/* Power supply model */
typedef enum { 
    DS1820_PWR_VCC, 
    DS1820_PWR_PARASITE 
} ds1820_pwr_t;

/* Family */
typedef enum { 
    DS1820_FAMILY_DS18S20 = 0x10,
    DS1820_FAMILY_DS18B20 = 0x28
} ds1820_family_t;


typedef struct {
    gpio_num_t pin;
    ds1820_family_t family;
    ds1820_pwr_t power;
    int resolution_bits;
    char rom[8];
    char scratchpad[9];
} ds1820_device_t;

ds1820_device_t *ds1820_init(gpio_num_t pin, const char *rom);
ds1820_err_t ds1820_read_temp(ds1820_device_t *dev, float *temp);
void ds1820_destroy(ds1820_device_t *dev);

#endif /* DS1820_H */
