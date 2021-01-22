# DS1820

Try to read a DS1820 1-wire temperature sensor.

## presence.c

Stand-alone file. It detects if any device is present by testing presence pulse.

Using Poor-Mans's Logic Analyzer:

    us , logic
    ..., 1
    057, 0
    583, 1
    610, 0
    714, 1

The master sets line down for 500us, then up again. After 30us slave takes the bus and keeps it down for 100us.

## ds1820.c / ds1820.h

Library to read temperature and ROM from a DS1820 or DS18S20 (but not DS18B20).

Functions:

### int ds1820_reset (gpio_num_t pin)

Reset the bus. Return 1 if a device is detected, 0 if not. `pin` argument is the GPIO pin which has the 1-wire device connected.

```c
if (ds1820_reset(PIN_1WIRE)) {
    ESP_LOGI(TAG, "Device present!");
}
```

### void ds1820_read_rom(gpio_num_t pin)

Read the ROM and print its contents as Info log.

```c
ds1820_read_rom(PIN_1WIRE);
```

Output:

    I (360) ds1820: Reading ROM data (Cmd 33h)
    I (374) ds1820: ROM data: 10 C4 54 E3 01 08 00 2E - CRC OK

### float ds1820_read_temp(gpio_num_t pin)

Reads the temperature.

```c
printf("Temperature is %.2fºC\n", ds1820_read_temp(PIN_1WIRE));
```

Output:

    Temperature is 20.25ºC

## main.c

Example program.
