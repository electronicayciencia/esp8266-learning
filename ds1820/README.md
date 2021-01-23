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

Only one device supported at a time.

Functions:

### int ds1820_reset (gpio_num_t pin)

Reset the bus. Return `DS1820_ERR_OK` if any device is detected, `DS1820_ERR_NODEVICE` if not. `pin` argument is the GPIO pin which has the 1-wire device connected.

Example:

```c
if (ds1820_reset(PIN_1WIRE) == DS1820_ERR_OK) {
    ESP_LOGI(TAG, "Device detected!");
}
```

### void ds1820_read_rom(gpio_num_t pin)

Read the ROM and print its contents as Info log.

Example:

```c
ds1820_read_rom(PIN_1WIRE);
```

Output:

    I (356) main: Device detected!
    I (361) ds1820: Reading ROM data (Cmd 33h)
    I (375) ds1820: ROM data: 10 C4 54 E3 01 08 00 2E - CRC OK

### int ds1820_read_temp(gpio_num_t pin, float *temp)

Reads the temperature.

Can return:

- `DS1820_ERR_OK` if everything was ok
- `DS1820_ERR_NODEVICE` if the device didn't answer
- `DS1820_ERR_BADCRC` if the CRC check failed (returned temperature might be incorrect)

Example:

```c
float temperature;
int result = ds1820_read_temp(PIN_1WIRE, &temperature);

if (result == DS1820_ERR_NODEVICE) {
    ESP_LOGW(TAG, "Device not present.");
}
else {
    printf("Temperature is %.2fºC (crc %s)\n", 
        temperature, 
        result == DS1820_ERR_OK ? "ok" : "error");
}
```

Output:

    Temperature is 19.88ºC (crc ok)

## main.c

Example program.
