# Just a simple ESPNOW broadcast receiver

Dump received data via uart. Can be used as ESPNOW->Serial bridge.

ESP-01S's blue LED (GPIO2) blinks on data reception.

## Output formats:

Readable:

```
I (447) espnow-rx: Waiting for data
Received 10 bytes from cc:50:e3:5d:6a:90
>|Data #166|<
Received 9 bytes from cc:50:e3:5d:6a:90
>|Data #61|<
...
```

Scriptable:

```
I (446) espnow-rx: Waiting for data
d cc50e35d6a90 10 Data #102
d cc50e35d6a90 10 Data #228
d cc50e35d6a90 9 Data #95
...
```

`d` means this is a line with incoming data. Fields are: MAC, length, message. 
The terminator is newline so it is not allowed in the message.
The separator is space to use it easily with bash/read.

## Raspberry Pi

UART0 in Raspberry Pi 3:
- GPIO15 -> RX

Activate using `raspi-config`. 
- Enable Harware UART.
- Disable serial console.

Raspberry Pi's UART does not accept 74880 bauds. So select 115200 as Monitor Baudrate in `make menuconfig`.

To test the receiver:

With serial terminal:

```console
# minicom -b 115200 -D /dev/ttyS0 -c on
```

Without serial terminal:

```console
# stty -F /dev/ttyS0 115200 raw
# cat /dev/ttyS0
>|cc50e35d6a90|10|Data #202
>|cc50e35d6a90|9|Data #90
```

Install mosquito and tools:

    # apt-get install mosquitto mosquitto-clients

