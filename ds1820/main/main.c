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

    // Configure 1WIRE pin as open drain output with pullup
    gpio_set_direction(PIN_1WIRE, GPIO_MODE_OUTPUT_OD);
    gpio_pullup_en(PIN_1WIRE);

    puts("DS1820 Test Program for ESP-IDF");
 
    if (ds1820_reset(PIN_1WIRE) == DS1820_ERR_OK) {
        ESP_LOGI(TAG, "Device detected!");
        ds1820_read_rom(PIN_1WIRE);
    }

    while (1) {
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

        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}
