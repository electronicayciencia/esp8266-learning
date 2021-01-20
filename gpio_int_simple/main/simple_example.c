/* gpio interrupt simplified example
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

static const char *TAG = "main";

/**
 * Brief:
 * This test reads a button from GPIO2 and print the event in log.
 *
 * GPIO status:
 * GPIO2:  input, pulled up, interrupt from rising edge and falling edge
 *
 * Test:
 * Connect a button between GPIO2 and ground
 */

#define PIN_INPUT          GPIO_NUM_2


static xQueueHandle gpio_evt_queue = NULL;

static void isr_pin_input_handler(void *arg) {
    uint32_t io_num = PIN_INPUT;
    xQueueSendFromISR(gpio_evt_queue, &io_num, NULL);
}

static void gpio_task_example(void *arg) {
    uint32_t io_num;
    static int64_t last_interrupt = 0;
    static int last_status = 0;

    for (;;) {
        if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            printf("GPIO[%d] intr, val: %d, elapsed: %d\n", 
                io_num, 
                gpio_get_level(io_num), 
                (int32_t) (esp_timer_get_time() - last_interrupt));

            if (gpio_get_level(io_num) == last_status) {
                ESP_LOGW(TAG, "Missed interruption.");
            }

            last_status = gpio_get_level(io_num);
            last_interrupt = esp_timer_get_time();
        }
    }
}

void app_main(void)
{


    /* Equivalent code, useful to configure multiple pins at once
    gpio_config_t io_conf;

    io_conf.intr_type = GPIO_INTR_ANYEDGE;
    io_conf.pin_bit_mask = (1ULL<<PIN_INPUT);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);
    */

    gpio_set_intr_type(PIN_INPUT, GPIO_INTR_ANYEDGE);
    gpio_set_direction(PIN_INPUT, GPIO_MODE_INPUT);
    gpio_pullup_en(PIN_INPUT);


    //create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    //start gpio task
    xTaskCreate(gpio_task_example, "gpio_task_example", 2048, NULL, 10, NULL);

    //install gpio isr service
    gpio_install_isr_service(0);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(PIN_INPUT, isr_pin_input_handler, NULL);

    while (1) {
        ESP_LOGI(TAG, "Running...");
        vTaskDelay(10000 / portTICK_RATE_MS);
    }
}


