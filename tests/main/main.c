/* 

Using the software PWM library.

*/
#include <stdio.h> // printf
#include "freertos/FreeRTOS.h" // portTICK_PERIOD_MS
#include "freertos/task.h" // vTaskDelay
#include "esp_log.h" // ESP_LOGI
#include "esp_system.h" // esp_random

#include "driver/pwm.h"

#define LOG_TAG "pwm"
#define PWM_0_IO 0
#define PWM_1_IO 2

// PWM period 1000us(1Khz), same as depth
#define PWM_PERIOD  1000

// pwm pin number
const uint32_t pin_num[] = {
	PWM_0_IO,
	PWM_1_IO,
};

// duties table, real_duty = duties[x]/PERIOD
uint32_t duties[] = {500, 500};

// phase table, delay = (phase[x]/360)*PERIOD
int16_t phases[] = {0, 0};

void app_main()
{
	pwm_init(PWM_PERIOD, duties, 2, pin_num);
	pwm_set_channel_invert(0b00000011); // LEDs are connected to positive.
	pwm_set_phases(phases);
	pwm_start();

	uint8_t duty_pct = 0;

	// Line to prevent flash hash error.
	ESP_LOGI(LOG_TAG, "Here is a random number: %d.", esp_random());

	while (1) {
		duty_pct++;
		
		//ESP_LOGI(LOG_TAG, "Duty: %d.", duty_pct);
		pwm_set_duty(0, duty_pct * PWM_PERIOD/100);
		pwm_start();

		
		if (duty_pct >=100) {
			duty_pct = 0;
		}

		vTaskDelay(20 / portTICK_RATE_MS);
	}
}
