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

#include "soft_lcd.h"


static void get_bus_times_task(void *pvParameters) {
    char token[EMT_TOKEN_LEN];
    bool is_last_token_valid = false;
    Bus buses[MAX_BUSES];

    while(true) {
        ESP_LOGI(TAG, "Waiting for WiFi...");
        wifi_wait_connected();
        ESP_LOGI(TAG, "WiFi available.");

        /* Login */
        while (
            is_last_token_valid == false && 
            emt_login(token, EMT_TOKEN_LEN) != ESP_OK) {

            ESP_LOGW(TAG, "Login call failed. Waiting for retry...");
            vTaskDelay(10000 / portTICK_PERIOD_MS);
        }
        ESP_LOGI(TAG, "Access token: %s", token);


        /* Get times */
        int n = emt_arrive_times(token, buses, MAX_BUSES);

        ESP_LOGI(TAG, "%d buses to arrive:", n);
        
        if (n < 0) {
            ESP_LOGW(TAG, "API call failed. Getting new token.");
            is_last_token_valid = false;
        }
        else {
            is_last_token_valid = true;

            int i;
            for (i = 0; i < n; i++) {
                ESP_LOGI(TAG, "Line %s at %d meters (%d seconds)",
                    buses[i].line,
                    buses[i].distance,
                    buses[i].time);
            }
        }
    

        for(int countdown = 10; countdown >= 0; countdown--) {
            ESP_LOGD(TAG, "%d...", countdown);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }

        ESP_LOGD(TAG, "Starting again!");
    }
}

void app_main() {
    //ESP_ERROR_CHECK( nvs_flash_init() );
    //wifi_initialise();
    //xTaskCreate(&get_bus_times_task, "get_bus_times_task", 8192, NULL, 5, NULL);


    /* Wait for startup GIPO party */ 
    vTaskDelay(500 / portTICK_PERIOD_MS);

    lcd_t *lcd = lcd_create(I2C_SCL_IO, I2C_SDA_IO, 0x27, 4);
 
    if (lcd == NULL) {
        ESP_LOGE(TAG, "Cannot set-up LCD.");
    }
    else {

        while(true) {
            //lcd_clear(lcd);
            //lcd_print(lcd, "ABCD");
            //vTaskDelay(2000 / portTICK_PERIOD_MS);

            lcd_clear(lcd);
            lcd_print(lcd, "This is line one.\n");
	        lcd_print(lcd, "This is line two.\n");
	        lcd_print(lcd, "This is line three.\n");
	        //lcd_print(lcd, "This is line four.\n");
            lcd_printf(lcd, "Random: %06d", rand());

            vTaskDelay(2000 / portTICK_PERIOD_MS);
        }
    }
}

