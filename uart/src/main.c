#include <osapi.h>
#include <user_interface.h>
#include "driver/uart.h"



void ICACHE_FLASH_ATTR user_init(void) {
	// 74880 is the same bitrate that startup logs.
	uart_init(BIT_RATE_74880, BIT_RATE_74880);
	
	os_printf("\n***System Initialized***\n");
	os_printf("Running at %dMHz.\n", system_get_cpu_freq());
	os_printf("This is a random number: %lu\n", os_random());
}


