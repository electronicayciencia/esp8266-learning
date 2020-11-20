#define TAG "beep"

#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/pwm.h"

#include "esp_bus.h"
#include "beep.h"



esp_err_t beep_init(uint32_t pin_num0) {
    #define DEFAULT_PERIOD 1000

    const uint32_t pin_num[] = {pin_num0};
    uint32_t duties[] = {DEFAULT_PERIOD/2};
    int16_t phases[] = {0};

	esp_err_t ret = pwm_init(DEFAULT_PERIOD, duties, 1, pin_num);
    if (ret == ESP_OK) {
        pwm_set_phases(phases);
    }
    return ret;
}

void beep(int freqhz, int lenms) {
    uint32_t duties[] = {5E5/freqhz};
    pwm_set_period_duties(1E6/freqhz, duties);
    pwm_start();
    vTaskDelay(lenms/portTICK_RATE_MS);
    pwm_stop(0);
}
