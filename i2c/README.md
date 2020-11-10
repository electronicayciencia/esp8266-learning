# I2C

Project to learn how to use I2C.

Files:

`i2c_scan_sequential.c.bak` Performs a scan of all addresses and write via UART when a device is found. Since SDK does not call stop after an error (not acknowledge from unused address), I must call stop command after each iteration.

`i2c_scan_parallel.c.bak` Performs a scan of all addresses and write via UART when a device is found but both tasks are done in parallel.

`main.c` Reads the LDR value from a PCF8591 module.
