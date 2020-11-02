# LED On Off

Very minimal project to blink a LED. Based on the PlatformIO examples.

**rf_cal_sensor** is a function needed the ESP SDK needs.

**user_init** is the main initialization function.

A timer must to be used because a busy delay for more than a few ms will trigger the ESP's hardware watchdog.

This example is just the bare minimal. No uart config, no wifi turnoff.
