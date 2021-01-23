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

#include "driver/gpio.h"

int ds1820_reset (gpio_num_t pin);
int ds1820_read_temp(gpio_num_t pin, float *temp);
int ds1820_read_rom(gpio_num_t pin);

#endif /* DS1820_H */
