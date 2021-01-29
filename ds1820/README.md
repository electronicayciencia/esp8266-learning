# DS1820

Try to read a DS1820 1-wire temperature sensor.

## presence.c

Stand-alone file. It detects if any device is present by testing presence pulse.

Using Poor-Man's Logic Analyzer:

    us , logic
    ..., 1
    057, 0
    583, 1
    610, 0
    714, 1

The master sets line down for 500us, then up again. After 30us slave takes the bus and keeps it down for 100us.

## DS1820 library

Library to read temperature and ROM from a DS1820 or DS18S20 (but not DS18B20).

### Features

- Unknown ROM operation (only one device in bus. Useful to discover the ROM number.)
- Multiple devices with known ROMs (ROM addressing mode)
- Parasite power detection (strong pull-up)

Bus scanning is not supported.

### Usage

#### Initialization

Initialization **from unknown ROM**. Simple when there are no more than one device connected.

```c
ds1820_device_t *dev = ds1820_init(PIN_1WIRE, DS1820_ROM_UNKNOWN);
```

Serial output:

    I (387) ds1820: At least 1 device detected!
    I (394) ds1820: Reading ROM data (Cmd 33h)
    I (409) ds1820: ROM data: 10 C4 54 E3 01 08 00 2E - CRC OK
    I (412) ds1820: Device family is DS1820 or DS18S20.
    I (428) ds1820: One o more devices are using parasitic power. Strong pull up active.

ROM, family and power mode are detected automatically.

Initialization **from known ROM**. Needed when you have multiple devices in the bus:

```c
#define ROM_1 "\x10\x8C\x67\xE3\x01\x08\x00\x30"

ds1820_device_t *device_1 = ds1820_init(PIN_1WIRE, ROM_1);
```

Return a pointer to a newly allocated DS1820 structure or **NULL** on error.

#### Reading Temperature

```c
float temperature;

ds1820_err_t result = ds1820_read_temp(dev, &temperature);

if (result == DS1820_ERR_OK) {
    printf("Temperature is %.2f\n", temperature);
}

else if (result == DS1820_ERR_EMPTYBUS) {
    ESP_LOGW(TAG, "No devices detected.");
}

else if (result == DS1820_ERR_NOANSWER) {
    ESP_LOGW(TAG, "Select device did not respond.");
}

else if (result == DS1820_ERR_BADCRC) {
    ESP_LOGW(TAG, "CRC Error.");
}
else {
    ESP_LOGE(TAG, "Unknown error.");
}
```

## read_temp.c

Example program.

Rename function `read_temp` to `app_main` to run it.

## Notes

unknown rom (1 device)
known rom (multiple devices)
parasitic power
