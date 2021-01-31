/* MQTEMP: Read the temperature from DS18B20 and report it via MQTT.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_system.h"
#include "mqtt_client.h"

#include "ds1820.h"
#include "wifi.h"

#define TAG  "MQTEMP"

// GPIO_NUM_2 do not seem to work 
#define PIN_1WIRE  GPIO_NUM_0

void app_main(void) {
    ESP_LOGI(TAG, 
        "MQTEMP: Read the temperature from DS18B20 and report it via MQTT.");

    wifi_init();

    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = CONFIG_BROKER_URL,
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_start(client);
    vTaskDelay(2000 / portTICK_RATE_MS); // use event loop instead

    ds1820_device_t *dev = ds1820_init(PIN_1WIRE, DS1820_ROM_UNKNOWN);

    if (dev == NULL) {
        ESP_LOGE(TAG, "Error");
        esp_mqtt_client_publish(client, "/mqtemp/log", "Bus error", 0, 0, 0);
        vTaskDelay(1000 / portTICK_RATE_MS);
        esp_restart();
    }

    while (1) {
        char buffer[100];
        float temperature;

        ds1820_err_t result = ds1820_read_temp(dev, &temperature);
        
        if (result == DS1820_ERR_OK) {
            snprintf(buffer, sizeof buffer, "mqtemp ontime=%ld,temp=%.3f",
                time(NULL), 
                temperature);
            printf("Sending: %s\n", buffer);
            esp_mqtt_client_publish(client, "/mqtemp/data", buffer, 0, 0, 0);
        }

        vTaskDelay(100 / portTICK_RATE_MS);
    }
}
