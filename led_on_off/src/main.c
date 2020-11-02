#include "osapi.h"
#include "user_interface.h"

os_timer_t ptimer;

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

void ICACHE_FLASH_ATTR user_init(void)
{
    

    //gpio_init();

    // Init uart
    //uart_init(74880, 74880);

    // Disable WiFi
    //wifi_set_opmode(NULL_MODE);

    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
    os_timer_disarm(&ptimer);
    os_timer_setfn(&ptimer, (os_timer_func_t *)blinky, NULL);
    os_timer_arm(&ptimer, 250, 1);

    //os_printf("Running...\n");
}


