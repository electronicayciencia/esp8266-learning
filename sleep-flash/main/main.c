#include <stdio.h> // printf
#include "freertos/FreeRTOS.h" // portTICK_PERIOD_MS
#include "freertos/task.h" // vTaskDelay
#include <driver/gpio.h> // gpio_set_level & others
#include <esp_log.h> // ESP_LOGI
#include <esp_sleep.h>

// LED is connected to Vcc.
#define LED_ON   0
#define LED_OFF  1


void task_flash_led() {
	while(true) {
		gpio_set_level(GPIO_NUM_2, LED_ON);
		vTaskDelay(50/portTICK_PERIOD_MS);
		gpio_set_level(GPIO_NUM_2, LED_OFF);

		vTaskDelay(1500/portTICK_PERIOD_MS);
	}
}


void app_main()
{
	gpio_config_t io_conf;
	io_conf.pin_bit_mask = GPIO_Pin_2;
	io_conf.mode = GPIO_MODE_OUTPUT;
	io_conf.intr_type = GPIO_INTR_DISABLE;
	gpio_config(&io_conf);

	gpio_set_level(GPIO_NUM_2, LED_OFF);

	xTaskCreate(task_flash_led, "flash_led", 1024, NULL, 5, NULL);
	ESP_LOGI("main","End. Go to bed.");
	
}
