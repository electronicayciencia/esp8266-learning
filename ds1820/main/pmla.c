/* Poor-man's Logic Analyzer
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "driver/gpio.h"

#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h" // high resolution timer

#define TAG        "main"
#define PIN_LOGIC_ANALYZER   GPIO_NUM_0
#define PIN_1WIRE            GPIO_NUM_2

/* 
PIN_LOGIC_ANALYZER generates interrupt on level change.
The interrupt handler retrives the time (us) when it happend.
The logic_analyzer_task prints out the level and time.

*/

typedef struct {
    bool level;
    uint64_t us;
} logic_analyzer_data_t;

static xQueueHandle logic_analyzer_queue = NULL;

static void logic_analyzer_isr(void *arg) {
    logic_analyzer_data_t data;
    data.us = esp_timer_get_time();
    data.level = gpio_get_level(PIN_LOGIC_ANALYZER);

    xQueueSendFromISR(logic_analyzer_queue, &data, NULL);
}

static void logic_analyzer_task(void *arg) {
    logic_analyzer_data_t data;
    static bool last_level = 0;

    for (;;) {
        if (xQueueReceive(logic_analyzer_queue, &data, portMAX_DELAY)) {
            if (data.level == last_level) {
                ESP_LOGW(TAG, "Missed interrupt!");
            }
            printf("%u, %d\n", (uint32_t) data.us, data.level);
            last_level = data.level;
        }
    }
}

void app_main(void)
{
    gpio_set_intr_type(PIN_LOGIC_ANALYZER, GPIO_INTR_ANYEDGE);
    gpio_set_direction(PIN_LOGIC_ANALYZER, GPIO_MODE_INPUT);
    gpio_pullup_en(PIN_LOGIC_ANALYZER);

    logic_analyzer_queue = xQueueCreate(10, sizeof(logic_analyzer_data_t));
    //start logic_analyzer_task to wait for queue events
    xTaskCreate(logic_analyzer_task, "la_task", 2048, NULL, 10, NULL);

    //install isr service
    gpio_install_isr_service(0);
    gpio_isr_handler_add(PIN_LOGIC_ANALYZER, logic_analyzer_isr, NULL);

    while (1) {
        //ESP_LOGI(TAG, "Running...");
        vTaskDelay(10000 / portTICK_RATE_MS);
    }
}


