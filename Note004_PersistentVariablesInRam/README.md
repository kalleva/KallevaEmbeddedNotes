# STM32. Creating Variables that persists between MCU reboots.

Sometimes you need to have some variable, that in the first start of your device is initialized to zero, but subsequent device resets without power down doesn't clear it back to zero.

Approach I use here is specific to the GNU Arm Embedded Toolchain.

[Code](https://github.com/kalleva/KallevaEmbeddedNotes/tree/master/Note004_PersistentVariablesInRam) for the post.

### Prerequisites:

- NUCLEO-L476RG board or any other device with STM32 MCU and the ability to print over UART.
- [Stm32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html)
- [Termite terminal](https://www.compuphase.com/software_termite.htm) to interact with device via UART.

### Setting things up

1. Create a project for NUCLEO-L476RG with STM32CubeIDE. When presented with CubeMX screen go to **Computing** > **CRC** and check **Activated** checkbox. We will use it later. After that go to code.

2. To get ```printf``` working over UART look up [Setting up UART communication with a device](https://kalleva.bearblog.dev/setting-up-uart-with-nucleo-l476rg-sending-strings-with-printf-and-recieving-lines-with-interrupts-to-ringbuffer/).

### Adding counter

1. Let's first add ```counter``` variable and see that it is reset to zero on device reboot.

```C
/* USER CODE BEGIN PV */
static uint32_t counter;
```

And inside the ```main``` function ```while (1)``` loop add:

```C
printf("counter: %lu\r\n", counter);
counter++;
HAL_Delay(1000);
```

If you run it you will get over UART:

```text
counter: 0
counter: 1
counter: 2
...
```

And if you reset device you will get counter going from 0 again.

```text
counter: 0
counter: 1
counter: 2
...
```

2. Lets change ```counter``` declaration and put it in ```.noinit``` section. 
```.noinit``` section is just what its name implies. For variables placed in this section there will be no setting to zero how it was for variables in ```.bss```.

Change ```counter``` declaration to this.

```C
uint32_t counter __attribute__((section(".noinit")));
```

After you compile and run this code you will get something like this. Counter is initialized to some value and incremented from there.

```text
counter: 553648168
counter: 553648169
counter: 553648170
counter: 553648171
```

If I reset the device, counter will retain it's value.

```text
counter: 553648228
counter: 553648229 <----- I did reset here
counter: 553648230
counter: 553648231
```

So at this point we are almost there. But what I wanted is to have counter initialized to zero in the first launch and retain its value in subsequent reboots.

3. Add another variable:

```C
uint32_t counter_crc __attribute__((section(".noinit")));
```

And also add this little function:

```C
static void persistent_variables_init(void)
{
  if (HAL_CRC_Calculate(&hcrc, &counter, 1) != counter_crc)
  {
    counter = 0;
    counter_crc = HAL_CRC_Calculate(&hcrc, &counter, 1);
  }
}
```

Put call to this function before you first use of ```counter``` variable.

And also every time you update ```counter``` in your code you will need to also update ```counter_crc```.

```C
counter++;
counter_crc = HAL_CRC_Calculate(&hcrc, &counter, 1);
```

What this approach does is on the first launch ```counter``` and ```counter_crc``` are set to some garbage value and CRC computer on ```counter``` will not match ```counter_crc```. In this case we initialize ```counter``` to zero. But on subsequent reboots value of CRC from ```counter``` and ```counter_crc``` will be equal so ```counter``` will not get initialized to zero.

```text
counter: 0
counter: 1
counter: 2
counter: 3 <----- I did reset here
counter: 4
counter: 5
counter: 6
```

This is it, ```counter``` behaves as we wanted it to.
