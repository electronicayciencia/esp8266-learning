# Experiments using ESP8266 microcontroller

- With PlatformIO:
  - **led_on_off**: blink a LED using NONOS SDK (deprecated)
  - **uart**: send information via serial interface using NONOS SDK (deprecated)
  - **led_on_off-rtos**: blink a LED using old RTOS SDK (somewhat deprecated)
- With MSYS32 ESP development environment for Windows:
  - **led_on_off-espidf**: blink a led using ESP-IDF style SDK.
  - **sleep-flash**: blink a led using ESP-IDF style SDK and RTOS tasks.

## Visual Studio Code

Tips to integrate VSC with ESP-IDF in Windows.

### Includes

You should add SDK and toolchain to include path. Create a new intellisense configuration (`c_cpp_properties.json`):

```json
"configurations": [
    {
        "name": "esp",
        "includePath": [
            "${default}",
            "${workspaceFolder}/build/include",
            "${workspaceFolder}/../../xtensa-lx106-elf/**",
            "${workspaceFolder}/../../ESP8266_RTOS_SDK/components/**",
            "${workspaceFolder}/../../ESP8266_RTOS_SDK/components/freertos/port/esp8266/include"
        ],
        "defines": [
            "_DEBUG",
            "UNICODE",
            "_UNICODE"
        ],
        "intelliSenseMode": "gcc-x64"
    }
```

### Terminal

To launch msys32 terminal edit your `workspace.json` and set this variables (correct your path):

```json
"settings": {
    ...
    "terminal.integrated.shell.windows": "D:\\tmp\\esp\\msys32\\usr\\bin\\bash.exe",
    "terminal.integrated.env.windows": {
        "CHERE_INVOKING": "1",
        "MSYSTEM": "MINGW32"
    },
    "terminal.integrated.shellArgs.windows": [
        "--login",
        "-i"
    ],
}
```
