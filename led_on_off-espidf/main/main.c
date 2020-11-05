#include <stdio.h> // printf
#include "freertos/FreeRTOS.h" // portTICK_PERIOD_MS
#include "freertos/task.h" // vTaskDelay
#include <driver/gpio.h> // gpio_set_level & others


void app_main()
{
	gpio_config_t io_conf;
	io_conf.pin_bit_mask = GPIO_Pin_2;
	io_conf.mode = GPIO_MODE_OUTPUT;
	gpio_config(&io_conf);

	printf("Hello.\n");

	while(true) {
		gpio_set_level(GPIO_NUM_2, 1);
		vTaskDelay(500/portTICK_PERIOD_MS);
		gpio_set_level(GPIO_NUM_2, 0);
		vTaskDelay(500/portTICK_PERIOD_MS);
	}

}
