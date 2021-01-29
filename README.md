# Experiments using ESP8266 microcontroller

- With PlatformIO:
  - **led_on_off**: blink a LED using NONOS SDK (deprecated)
  - **uart**: send information via serial interface using NONOS SDK (deprecated)
  - **led_on_off-rtos**: blink a LED using old RTOS SDK (somewhat deprecated)
- With MSYS32 ESP development environment for Windows:
  - **led_on_off-espidf**: blink a led using ESP-IDF style SDK.
  - **sleep-flash**: blink a led using ESP-IDF style SDK and RTOS tasks.

## ds1820

Library to read a 1820 sensor.

## esp_bus

Read about it in (spanish) [Avisador personal de autobús con ESP8266](https://www.electronicayciencia.com/2021/01/avisador-personal-autobus-con-esp8266.html).

Includes:

- Wifi
- LCD 20x04
- PWM
- API REST using SSL
- FreeRTOS task management

