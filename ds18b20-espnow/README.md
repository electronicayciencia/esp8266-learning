# ESPNOW thermometer

Read a ds18b20, send the value using ESP broadcast, deep sleep for a while.

The message sent is "temp=XX.X" where XXX is the temperature in ºC.

To reduce boot time:
 - bootloader -> verbosity: warn
 - compiler options -> optimization: release
 - components -> log output -> default log verbosity: warning

Disable *'nano' formatting options* in menuconfig to enable floating point output.
