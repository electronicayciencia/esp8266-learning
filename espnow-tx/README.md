# Just a simple ESPNOW broadcaster

Send a random value number to broadcast using ESP-NOW. Then deep sleep for 1s.

To reduce boot time:
 - bootloader -> disable jtag io
 - bootloader -> verbosity: warn
 - compiler options -> optimization: release

