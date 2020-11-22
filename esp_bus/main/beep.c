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

/* Create different kind of alerts */
static void bg_alert_task(void *pvParameters) {
    int type = (uint32_t) pvParameters;

    ESP_LOGD(TAG, "Launched type %d alert.", (uint32_t) pvParameters);

    /* bip bip bip bip - x3 */
    if (type == 1) {
        int i;
        for (i = 0; i < 3; i++) {
            int j;
            for (j = 0; j < 4; j++) {
                beep(800, 100);
                vTaskDelay(100 / portTICK_PERIOD_MS);
            }
            vTaskDelay(300 / portTICK_PERIOD_MS);
        }
    }

    /* beeeeep beeeeep - x2 */
    else if (type == 2) {
        int i;
        for (i = 0; i < 2; i++) {
            int j;
            for (j = 0; j < 2; j++) {
                beep(1000, 750);
                vTaskDelay(200 / portTICK_PERIOD_MS);
            }
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }

    else {
        ESP_LOGW(TAG, "Unknown alert type: %d", type);
    }

    vTaskDelete(NULL);
}

/* Launch a background task for beeping */
esp_err_t alert(int alert_type) {
    long ret = xTaskCreate(
        &bg_alert_task, 
        "beep", 
        2048, 
        (void *) alert_type,
        5,
        NULL);

    if(ret == pdPASS) {
        return ESP_OK;
    }
    else {
        return ESP_ERR_NO_MEM;
    }
}
