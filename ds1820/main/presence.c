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
#include "esp_timer.h" // high resolution timer

#define TAG        "main"
#define PIN_1WIRE  GPIO_NUM_0

#define LOW  0
#define HIGH 1

// Bus recovery time
#define TRECOVERY 2

// Device timing parameters (see DS1820 datasheet)
#define TRSTL_MIN 480
#define TPDHIGH_MAX 60

void app_main(void)
{
    esp_log_level_set(TAG, ESP_LOG_DEBUG);

    // Configure 1WIRE pin as open drain output with pullup
    gpio_set_direction(PIN_1WIRE, GPIO_MODE_OUTPUT_OD);
    gpio_pullup_en(PIN_1WIRE);

    while (1) {
        vTaskDelay(5000 / portTICK_RATE_MS);

        // set the bus low for more than TRSTL_MIN us.
        gpio_set_level(PIN_1WIRE, LOW);
        ets_delay_us(TRSTL_MIN);
        gpio_set_level(PIN_1WIRE, HIGH);

        // wait for bus recovery, read again, it must be high 
        ets_delay_us(TRECOVERY);
        if (gpio_get_level(PIN_1WIRE) == LOW) {
            ESP_LOGE(TAG, "Bus did not back to high!");
            continue;
        }

        // wait a few moments, read again,
        // if there is a device, we must read low.
        ets_delay_us(TPDHIGH_MAX);
        if (gpio_get_level(PIN_1WIRE) == LOW) {
            ESP_LOGI(TAG, "Presence pulse detected!");
        }
        else {
            ESP_LOGD(TAG, "No device present.");
        }
    }
}
