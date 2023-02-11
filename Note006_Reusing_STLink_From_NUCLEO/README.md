# Reusing ST-Link From NUCLEO

One time I found that I needed to debug STM32 board but didn't have ST-Link dongle on me.
But I had spare NUCLEO laying around, so I figured out I can get ST-Link from it.
And sure enough if you look at [UM1724 User manual STM32 Nucleo-64 boards](st.com/resource/en/user_manual/um1724-stm32-nucleo64-boards-mb1136-stmicroelectronics.pdf) you will see that there is **CN4** Debug SWD Connector.
To use it you need to take **CN2** jumpers OFF, so board will be configured in ST-Link mode and not in NUCLEO mode.

This is the **CN4** pinout from Table 5. of UM1724:

```Text
1   VDD_TARGET          VDD from application
2   SWCLK               SWD clock
3   GND                 ground
4   SWDIO               SWD data input/output
5   NRST                RESET of target STM32
6   SWO                 Reserved
```

I found out that it is pretty reliable ST-Link and this way you can get around some problems with cheap ST-Link clones, because all software from STM treats is a valid ST-Link.
