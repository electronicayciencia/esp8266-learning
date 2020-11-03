#include "esp_common.h"
#include "freertos/task.h"
#include "gpio.h"


void task_blink(void* ignore) {
	while(true) {
		GPIO_OUTPUT_SET(2, 0);
		vTaskDelay(100/portTICK_RATE_MS);
		GPIO_OUTPUT_SET(2, 1);
		vTaskDelay(100/portTICK_RATE_MS);
	}
	vTaskDelete(NULL);
}

void user_init(void) {
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
	xTaskCreate(&task_blink, "startup", 2048, NULL, 1, NULL);
}

