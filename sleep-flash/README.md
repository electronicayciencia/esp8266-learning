# Sleep flash

Go to sleep, wake every second to flash a LED for 0.1s and sleep again.
ESP-IDF style (v3.3 stable)

Compile under ESP tools environment (Windows or Linux)

- make menuconfig (select serial port and flash size)
- make
- make flash
- make monitor (or make simple_monitor)

RTOS tasks must **infinite loop or terminate**. Otherwise...

    Guru Meditation Error: Core  0 panic'ed (InstrFetchProhibited). Exception was unhandled.
    Core 0 register dump:
    PC      : 0x00000000  PS      : 0x00000030  A0      : 0x00000000  A1      : 0x3ffeba00
    A2      : 0x00000020  A3      : 0x00000000  A4      : 0x40105870  A5      : 0x40105870
    A6      : 0xffffffff  A7      : 0x00000077  A8      : 0x00000000  A9      : 0x00000000
    A10     : 0x00000000  A11     : 0x00000000  A12     : 0x00000000  A13     : 0x00000000
    A14     : 0x00000000  A15     : 0x00000000  SAR     : 0x0000001e  EXCCAUSE: 0x00000014


[ESP8266_RTOS_SDK (IDF Style) Programming Guide](https://docs.espressif.com/projects/esp8266-rtos-sdk/en/latest/index.html)


Calls to ESP_LOGI do not need CRLF.

Current is 13mA.