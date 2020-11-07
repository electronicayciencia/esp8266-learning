#include <stdio.h> // printf
#include "freertos/FreeRTOS.h" // portTICK_PERIOD_MS
#include "freertos/task.h" // vTaskDelay
#include <driver/gpio.h> // gpio_set_level & others
#include <esp_log.h> // ESP_LOGI
#include <esp_sleep.h>

// LED is connected to Vcc.
#define LED_ON   0
#define LED_OFF  1

#define PIN_LED_1 GPIO_NUM_0
#define PIN_LED_2 GPIO_NUM_2

#define PERIOD_1_MS 1000
#define PERIOD_2_MS 150


void task_flash_led_1() {
	while(true) {
		gpio_set_level(PIN_LED_1, LED_ON);
		vTaskDelay(PERIOD_1_MS/2/portTICK_PERIOD_MS);
		gpio_set_level(PIN_LED_1, LED_OFF);
		vTaskDelay(PERIOD_1_MS/2/portTICK_PERIOD_MS);
	}
}

void task_flash_led_2() {
	while(true) {
		gpio_set_level(PIN_LED_2, LED_ON);
		vTaskDelay(PERIOD_2_MS/2/portTICK_PERIOD_MS);
		gpio_set_level(PIN_LED_2, LED_OFF);
		vTaskDelay(PERIOD_2_MS/2/portTICK_PERIOD_MS);
	}
}


void app_main()
{
	gpio_config_t io_conf;
	io_conf.pin_bit_mask = (1 << PIN_LED_1) | (1 << PIN_LED_2);
	io_conf.mode = GPIO_MODE_OUTPUT;
	io_conf.intr_type = GPIO_INTR_DISABLE;
	gpio_config(&io_conf);

	gpio_set_level(GPIO_NUM_2, LED_OFF);

	xTaskCreate(task_flash_led_1, "flash_led_1", 1024, NULL, 5, NULL);
	xTaskCreate(task_flash_led_2, "flash_led_2", 1024, NULL, 5, NULL);
	ESP_LOGI("main","End. Go to bed.");
	
}
