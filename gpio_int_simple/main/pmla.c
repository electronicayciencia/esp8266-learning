/* PMLA: Poor Man's Logic Analyzer
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
#define PIN_PMLA   GPIO_NUM_2

/* 
PIN_PMLA generates interrupt on level change.
The interrupt handler retrives the time (us) when it happend.
The pmla_task prints out the level and time.

*/

typedef struct {
    bool level;
    uint64_t us;
    uint32_t gpio;
} pmla_data_t;

static xQueueHandle pmla_queue = NULL;

static void pmla_isr(void *arg) {
    pmla_data_t data;
    data.us = esp_timer_get_time();
    data.level = gpio_get_level(PIN_PMLA);
    data.gpio = (uint32_t) arg;

    xQueueSendFromISR(pmla_queue, &data, NULL);
}

static void pmla_task(void *arg) {
    pmla_data_t data;
    static bool last_level = 0;

    for (;;) {
        if (xQueueReceive(pmla_queue, &data, portMAX_DELAY)) {
            // Missed interrupt detection only works for one pin
            if (data.level == last_level) {
                ESP_LOGW("PMLA", "Missed interrupt!");
            }
            printf("%u, %u, %d\n", (uint32_t) data.us, data.gpio, data.level);
            last_level = data.level;
        }
    }
}

void app_main(void)
{
    gpio_set_intr_type(PIN_PMLA, GPIO_INTR_ANYEDGE);
    gpio_set_direction(PIN_PMLA, GPIO_MODE_INPUT);
    gpio_pullup_en(PIN_PMLA);

    pmla_queue = xQueueCreate(10, sizeof(pmla_data_t));
    //start pmla_task to wait for queue events
    xTaskCreate(pmla_task, "pmla_task", 2048, NULL, 10, NULL);

    //install isr service
    gpio_install_isr_service(0);
    gpio_isr_handler_add(PIN_PMLA, pmla_isr, NULL);

    while (1) {
        //ESP_LOGI(TAG, "Running...");
        vTaskDelay(10000 / portTICK_RATE_MS);
    }
}


