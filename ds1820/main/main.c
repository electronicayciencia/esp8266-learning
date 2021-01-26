/* Detect DS1820 presence pulse
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"

#include "esp_log.h"
#include "esp_system.h"

#include "ds1820.h"

#define TAG  "main"
#define LOW  0
#define HIGH 1

#define PIN_1WIRE  GPIO_NUM_0

void app_main(void) {
    //esp_log_level_set(TAG, ESP_LOG_DEBUG);

    puts("DS1820 Test Program for ESP-IDF");

    ds1820_device_t dev = ds1820_init(PIN_1WIRE, DS1820_ROM_UNKNOWN);

    if (ds1820_reset(PIN_1WIRE) == DS1820_ERR_OK) {
        ESP_LOGI(TAG, "Device detected!");
        ds1820_read_rom(PIN_1WIRE);
    }

    while (1) {
        float temperature;
        int result = ds18b20_read_temp(PIN_1WIRE, &temperature);
        
        if (result == DS1820_ERR_NODEVICE) {
            ESP_LOGW(TAG, "Device not present.");
        }
        else {
            printf("Temperature is %.2f (crc %s)\n", 
                temperature, 
                result == DS1820_ERR_OK ? "ok" : "error");
        }

        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}
