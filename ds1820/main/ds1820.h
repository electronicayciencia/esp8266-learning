/* 
    Simple library for DS1820 for ESP-IDF

    Electronicayciencia 22/01/2021
*/
#ifndef DS1820_H
#define DS1820_H

#include "driver/gpio.h"

int ds1820_reset (gpio_num_t pin);
float ds1820_read_temp(gpio_num_t pin);
void ds1820_read_rom(gpio_num_t pin);

#endif /* DS1820_H */
