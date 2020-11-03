# LED On Off using RTOS

Non OS SDK is discontinued:

> Support Policy for ESP8266 NonOS (Starting from December 2019)
>
> - We will not add any new features to the ESP8266 NonOS SDK.
> - We will only fix critical bugs in the ESP8266 NonOS SDK.
> - It is suggested that the ESP8266_RTOS_SDK, instead of ESP8266 NonOS > SDK, be used for your projects.

Some other tasks are running on background while the LED is blinking:

    ets Jan  8 2013,rst cause:2, boot mode:(3,6)

    load 0x40100000, len 31360, room 16
    tail 0
    chksum 0x07
    load 0x3ffe8000, len 2128, room 8
    tail 8
    chksum 0xb2
    load 0x3ffe8850, len 492, room 0
    tail 12
    chksum 0xe3
    csum 0xe3
    OS SDK ver: 1.5.0-dev(caff253) compiled @ Oct 23 2017 17:42:20
    phy ver: 1055_1, pp ver: 10.7

    rf cal sector: 251
    tcpip_task_hdl : 3ffef620, prio:10,stack:512
    idle_task_hdl : 3ffef6c0,prio:0, stack:384
    tim_task_hdl : 3fff1e78, prio:2,stack:512
    mode : softAP(ce:50:e3:5d:6a:90)
    dhcp server start:(ip:192.168.4.1,mask:255.255.255.0,gw:192.168.4.1)
    add if1
    bcn 100



