/* 
    Simple library for DS1820 for ESP-IDF

    Electronicayciencia 22/01/2021
*/
#ifndef DS1820_H
#define DS1820_H

/* Errors */
#define DS1820_ERR_OK 0
#define DS1820_ERR_BADCRC 1
#define DS1820_ERR_NODEVICE 2



/* Misc */
#define DS1820_ROM_UNKNOWN NULL


#include "driver/gpio.h"

typedef int ds1820_err_t;

typedef struct {
    gpio_num_t pin;
    int family;
    int power;
    char rom[8];
    char scratchpad[9];
} ds1820_device_t;

ds1820_device_t *ds1820_init(gpio_num_t pin, const char *rom);
ds1820_err_t ds1820_reset(ds1820_device_t *dev);
ds1820_err_t ds1820_read_rom(ds1820_device_t *dev);
ds1820_err_t ds1820_read_temp(ds1820_device_t *dev, float *temp);
ds1820_err_t ds18b20_read_temp(ds1820_device_t *dev, float *temp);

#endif /* DS1820_H */
