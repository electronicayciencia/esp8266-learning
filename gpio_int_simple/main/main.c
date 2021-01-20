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

#define GPIO_INPUT          2
#define GPIO_INPUT_MASK    (1ULL<<GPIO_INPUT) 

static xQueueHandle gpio_evt_queue = NULL;

static void gpio_isr_handler(void *arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

static void gpio_task_example(void *arg)
{
    uint32_t io_num;

    for (;;) {
        if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            ESP_LOGI(TAG, "GPIO[%d] intr, val: %d\n", io_num, gpio_get_level(io_num));
        }
    }
}

void app_main(void)
{
    gpio_config_t io_conf;

    io_conf.intr_type = GPIO_INTR_ANYEDGE;
    io_conf.pin_bit_mask = GPIO_INPUT_MASK;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    //create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    //start gpio task
    xTaskCreate(gpio_task_example, "gpio_task_example", 2048, NULL, 10, NULL);

    //install gpio isr service
    gpio_install_isr_service(0);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(GPIO_INPUT, gpio_isr_handler, (void *) GPIO_INPUT);

    while (1) {
        vTaskDelay(10000 / portTICK_RATE_MS);
        printf("Waiting, push a button...\n");
    }
}


