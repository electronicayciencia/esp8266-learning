#define TAG "beep"

#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "esp_event_loop.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "driver/pwm.h"

#include "esp_bus.h"
#include "beep.h"


#define BEEP_NOW  BIT0
static EventGroupHandle_t beep_event_group;

/* Non-blocking beep */
void beep() {
    xEventGroupSetBits(beep_event_group, BEEP_NOW);
}

void beep_wait_to_beep() {
    xEventGroupWaitBits(beep_event_group, BEEP_NOW, true, true, portMAX_DELAY);
}

void beep_init() {
    #define PWM_PERIOD  1E6/BEEP_FREQ

    const uint32_t pin_num[] = {BEEPER_IO};
    uint32_t duties[] = {PWM_PERIOD/2};
    int16_t phases[] = {0};

	pwm_init(PWM_PERIOD, duties, 1, pin_num);
    pwm_set_phases(phases);
}

void beep_once() {
    pwm_start();
    vTaskDelay(BEEP_MS / portTICK_RATE_MS);
    pwm_stop(0);
}
