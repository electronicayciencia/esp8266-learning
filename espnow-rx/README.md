# Just a simple ESPNOW broadcast receiver

Dump received data via uart. Can be used as ESPNOW->Serial bridge.

Output formats:

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
>|cc50e35d6a90|10|Data #102
>|cc50e35d6a90|10|Data #228
>|cc50e35d6a90|9|Data #95
...
```

Where `>|` is a mark of line with incoming data, MAC, len, message.

Raspberry Pi UART does not accept 74880 bauds. So select 115200 as Monitor Baudrate in menuconfig.


