#include "osapi.h"
#include "user_interface.h"

os_timer_t ptimer; // needs to be global

void blinky(void *arg)
{
	static uint8_t state = 0;

	state ^= 1;
	if (state) {
		GPIO_OUTPUT_SET(2, 1);
	} else {
		GPIO_OUTPUT_SET(2, 0);
	}
}

void ICACHE_FLASH_ATTR user_init(void) {
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
	os_timer_disarm(&ptimer);
	os_timer_setfn(&ptimer, (os_timer_func_t *)blinky, NULL);
	os_timer_arm(&ptimer, 100, 1);
}


