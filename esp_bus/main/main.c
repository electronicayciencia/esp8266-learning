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
#include "time.h"

#include "esp_bus.h"
#include "config.h"
#include "wifi.h"
#include "http_api.h"
#include "lcd.h"
#include "beep.h"



#define NOBUS_LINE "         -          "
#define NOBUS_LINE_LEN 20

/* lcd_ram maps the LCD character memory.
   Each part of the program alters its reserved space.
   Every second, the contets of this variable
   are transfer to the physical LCD device. */
char lcd_ram[LCD_LEN+1];
time_t last_read_time = 0;


/* Copy LCD RAM to physical LCD in a periodic way */
void update_lcd_physical_task(void *pvParameters) {
    
    memset(lcd_ram,' ', LCD_LEN);
    //                          12345678901234567890
    //                         |--------------------|
    memcpy(lcd_ram,            "Bus#    Dist  Tiempo", 20);
    memcpy(lcd_ram+LCD_LINE_2, NOBUS_LINE, NOBUS_LINE_LEN);   
    memcpy(lcd_ram+LCD_LINE_3, NOBUS_LINE, NOBUS_LINE_LEN);   
    memcpy(lcd_ram+LCD_LINE_4, "Hace --s  L--- P----", 20);

    format_busstop(atoi(BUS_STOP), lcd_ram+LCD_LINE_4+16);
    format_busline(BUS_LINE, lcd_ram+LCD_LINE_4+11);

    lcd_new_data();

    while(true) {
        lcd_wait_new_data();
        update_lcd_physical(lcd_ram);
    }
}

/* Update the age of the shown data in seconds */
void update_elapsed_task(void *pvParameters) {
    while(true) {
        format_elapsed(
            time(NULL)-last_read_time, 
            lcd_ram+LCD_LINE_4+5);
        lcd_new_data();
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}


void update_bus_times_task(void *pvParameters) {
    char token[EMT_TOKEN_LEN];
    bool is_last_token_valid = false;
    bus_t buses[MAX_BUSES];

    int last_bus0_distance = 0; // keep track to fire alerts

    while(true) {
        ESP_LOGI(TAG, "Retrieving EMT data.");
        
        ESP_LOGD(TAG, "Waiting for WiFi...");
        wifi_wait_connected();
        ESP_LOGD(TAG, "WiFi available.");

        /* Login */
        while (
            is_last_token_valid == false && 
            emt_login(token, EMT_TOKEN_LEN) != ESP_OK) {

            ESP_LOGW(TAG, "Login call failed. Waiting for retry...");
            vTaskDelay(10000 / portTICK_PERIOD_MS);
        }
        ESP_LOGD(TAG, "Access token: %s", token);


        /* Get times */
        int n = emt_arrive_times(token, buses, MAX_BUSES);

        last_read_time = time(NULL);

        ESP_LOGI(TAG, "%d buses to arrive:", n);
        
        if (n < 0) {
            ESP_LOGW(TAG, "API call failed. Getting new token.");
            is_last_token_valid = false;
            continue;
        }

        is_last_token_valid = true;

        int i;
        for (i = 0; i < n; i++) {
            ESP_LOGI(TAG, "Line %s at %d meters (%d seconds)",
                buses[i].line,
                buses[i].distance,
                buses[i].time);
        }

    
        /* Update LCD zones for times and distances */
        if (n > 0) {
            format_busnumber(buses[0].number, lcd_ram+LCD_LINE_2);
            format_distance(buses[0].distance, lcd_ram+LCD_LINE_2+7);
            format_time(buses[0].time, lcd_ram+LCD_LINE_2+14);
        }
        else {
            memcpy(lcd_ram+LCD_LINE_2, NOBUS_LINE, NOBUS_LINE_LEN);
        }

        if (n > 1) {
            format_busnumber(buses[1].number, lcd_ram+LCD_LINE_3);
            format_distance(buses[1].distance, lcd_ram+LCD_LINE_3+7);
            format_time(buses[1].time, lcd_ram+LCD_LINE_3+14);
        }
        else {
            memcpy(lcd_ram+LCD_LINE_3, NOBUS_LINE, NOBUS_LINE_LEN);
        }

        lcd_new_data();

        
        /* Alert when the bus is near */

        if (
            last_bus0_distance > alert_1_meters && 
            buses[0].distance < alert_1_meters) {

                ESP_LOGD(TAG, "Fired alert 1.");
                alert(1);
        }

        else if (
            last_bus0_distance > alert_2_meters && 
            buses[0].distance < alert_2_meters) {

                ESP_LOGD(TAG, "Fired alert 2.");
                alert(2);
        }

        last_bus0_distance = buses[0].distance;

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}



void app_main() {
    ESP_LOGI(TAG, "Welcome to Electronica y Ciencia's Bus Personal Timer.");

    ESP_ERROR_CHECK(nvs_flash_init());
    wifi_initialise();

    vTaskDelay(500 / portTICK_PERIOD_MS); // Wait for the GPIO startup party

    ESP_ERROR_CHECK(lcd_initialise(I2C_SCL_IO, I2C_SDA_IO)); // if it fails, it may be connected to the programmer
    ESP_ERROR_CHECK(beep_init(BEEPER_IO));

    xTaskCreate(&update_lcd_physical_task, "update_lcd_physical_task", 2048, NULL, 5, NULL);
    xTaskCreate(&update_bus_times_task, "update_bus_times_task", 8192, NULL, 4, NULL);
    xTaskCreate(&update_elapsed_task, "update_elapsed_task", 2048, NULL, 4, NULL);
}
