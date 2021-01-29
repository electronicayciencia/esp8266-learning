/* MQTEMP: Read the temperature from DS18B20 and report it via MQTT.
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

#define ROM_1 "\x10\x8C\x67\xE3\x01\x08\x00\x30"
#define ROM_2 "\x10\xC4\x54\xE3\x01\x08\x00\x2E"
#define ROM_3 "\x28\xFF\x8C\x7C\x81\x16\x03\x75"

// GPIO_NUM_2 do not seem to work 
#define PIN_1WIRE  GPIO_NUM_0

void app_main(void) {
    //esp_log_level_set(TAG, ESP_LOG_DEBUG);

    puts("MQTEMP: Read the temperature from DS18B20 and report it via MQTT.");

    ds1820_device_t *dev = ds1820_init(PIN_1WIRE, DS1820_ROM_UNKNOWN);
    //ds1820_device_t *dev = ds1820_init(PIN_1WIRE, ROM_1);


    if (dev == NULL) {
        ESP_LOGE(TAG, "Error");
        vTaskDelay(1000 / portTICK_RATE_MS);
        esp_restart();

    }

    while (1) {
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

        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}
