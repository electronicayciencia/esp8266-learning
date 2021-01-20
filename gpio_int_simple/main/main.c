/* gpio interrupt frequency counter
   Input is GPIO2.
   Performance is good until about 90kHz.
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
#define PIN_INPUT   GPIO_NUM_2

uint32_t counter;

static void isr_pin_input_handler(void *arg) {
    counter++;
}

void app_main(void) {
    counter = 0;

    gpio_set_direction(PIN_INPUT, GPIO_MODE_INPUT);
    gpio_set_intr_type(PIN_INPUT, GPIO_INTR_POSEDGE); // one of the edges
    gpio_pullup_en(PIN_INPUT);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(PIN_INPUT, isr_pin_input_handler, NULL);

    while (1) {
        vTaskDelay(1000 / portTICK_RATE_MS);
        printf("Frequency: %d Hz\n", counter);
        counter = 0;
    }
}
