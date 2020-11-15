/* 

EMT bus monitor.

Electronicayciencia 2020/11/13.

Based on https_request example.

*/
#define TAG "main"

#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"


#include "esp_bus.h"
#include "config.h"
#include "wifi.h"
#include "http_api.h"


static void https_get_task(void *pvParameters) {
    char token[EMT_TOKEN_LEN];

    while(1) {
        ESP_LOGI(TAG, "Waiting for WiFi...");
        wifi_wait_connected();
        ESP_LOGI(TAG, "Connected to AP!");

        
        if (emt_login(token, EMT_TOKEN_LEN) == ESP_OK) {
            ESP_LOGI(TAG, "Got access token: %s", token);

            Bus buses[MAX_BUSES];
            int n = emt_arrive_times(token, buses, MAX_BUSES);
            ESP_LOGI(TAG, "%d buses to arrive:", n);
            
            
            int i;
            for (i = 0; i < n; i++) {
                ESP_LOGI(TAG, "Line %s at %d meters (%d seconds)",
                    buses[i].line,
                    buses[i].distance,
                    buses[i].time);
            }
        }
        else {
            ESP_LOGE(TAG, "Login call failed.");
        }



        for(int countdown = 10; countdown >= 0; countdown--) {
            ESP_LOGI(TAG, "%d...", countdown);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        ESP_LOGI(TAG, "Starting again!");
    }
}

void app_main() {
    ESP_ERROR_CHECK( nvs_flash_init() );
    wifi_initialise();
    xTaskCreate(&https_get_task, "https_get_task", 8192, NULL, 5, NULL);
}

