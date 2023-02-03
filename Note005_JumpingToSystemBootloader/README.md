# Jumping To System Bootloader on STM32

You can enter bootloader on STM32 MCU by pulling BOOT pin to Vdd and resetting the device.
When MCU loads it will sample BOOT pin and it is pulled to Vdd it will load executable code from system memory instead of loading from FLASH memory.

But sometimes you want the ability to from user application to bootloader without using BOOT pin.
For example you just doesn't have access to BOOT pin on your board.
In this post I will show how to do it for NUCLEO-L476RG.
[Code](https://github.com/kalleva/KallevaEmbeddedNotes/tree/master/Note005_JumpingToSystemBootloader) for this post.

## Prerequisites

1. [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html)
2. [STM32CubeProgrammer](https://www.st.com/en/development-tools/stm32cubeprog.html)

## Building example

You can read about stm32 bootloader in [AN2606 STM32 microcontroller system memory boot mode](https://www.st.com/resource/en/application_note/cd00167594-stm32-microcontroller-system-memory-boot-mode-stmicroelectronics.pdf).
At least you need to look up in it start address of System memory. For STM32L47xxx devices it starts at ```0x1FFF0000```.

My plan is to build an application that will be jumping to bootloader after user presses button on the NUCLEO.

1. Start a new project with STM32CubeIDE. Select preconfigured peripherals option when prompted.
But then go to GPIO section of the device configuration and change the B1 pin (PC13) to simple Input.
And after that generate code.

2. First lets write simple debounce function for our button, because it's always nice to have some debounce on your button for stability.
This is simple debounce function that I took from this post http://www.ganssle.com/debouncing-pt2.htm and use regularly.

```C
bool debounce_button(void)
{
    static uint16_t state = 0;
    state = (state << 1) | !HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) | 0xE000;
    return state == 0xF000;
}
```

3. Before jumping to bootloader you need properly deinitialize all peripherals because they can interfere with proper execution of bootloader.
Instead of going through all of them, it is simpler to set some magic variable on the button press that will not be cleared on MCU reset and reset MCU.

Here we declare a variable that will hold magic value.

```C
uint32_t jump_to_boot_persistent __attribute__((section(".noinit")));
```

When button is pressed we set this variable to magic value and initiate reset.

```C
if (debounce_button())
{
  jump_to_boot_persistent = JUMP_TO_BOOT_VALUE;
  NVIC_SystemReset();
}
```

And with this we can add check if variable is equal to magic value at the very beginning of the ```main``` function before any peripherals get initialized.
If it is equal to magic value we will set things up and jump to bootloader.

```
if (jump_to_boot_persistent == JUMP_TO_BOOT_VALUE)
  initiate_jump_to_bootloader();
```

4. Performing jump to bootloader

```C
#define UART_BOOTLOADER_ADDRESS 0x1FFF0000

void (*sys_mem_boot_jump_func)(void);

void initiate_jump_to_bootloader(void)
{
  uint32_t jump_address = UART_BOOTLOADER_ADDRESS + 4;
  sys_mem_boot_jump_func = (void (*) (void)) (*((uint32_t *) jump_address));
  HAL_RCC_DeInit();
  SysTick->CTRL = 0;
  SysTick->LOAD = 0;
  SysTick->VAL = 0;
  __disable_irq();
  __DSB();__HAL_SYSCFG_REMAPMEMORY_SYSTEMFLASH();
  __DSB();__ISB();
  __set_MSP(*(__IO uint32_t*)UART_BOOTLOADER_ADDRESS);
  sys_mem_boot_jump_func();
}
```

As I said for this MCU bootloader is located at ```0x1FFF0000```.
We need to define function pointer ```sys_mem_boot_jump_func``` through which we will start execute bootloader code.
After that we set the address from which we should execute code - ```UART_BOOTLOADER_ADDRESS + 4```.
```+ 4``` is here because in first 4 bytes of System memory resides stack pointer.

We set code at that address for execution through ```sys_mem_boot_jump_func```.

Next we perform some MCU cleanup so bootloader will work correctly,
we reset RCC to its default state and disable and reset to default values SysTick timer and finally disable interrupts.

After that we remap system memory to ```0x0000 0000```.
Because memory need to be remapped to ```0x0000 0000``` before we start execute code from it.
Flash gets also remapped from ```0x0800 0000``` before application starts from it.

Next we flush processor pipeline with __ISB() so next instruction will be fetched from cache or memory.

And now all we need is to set stack pointer and we can perform jump to bootloader code.

If you compile and load this NUCLEO, after you push the B1 button MCU will get in bootloader mode.
You can check it if you start STM32CubeProgrammer and connect to board through UART interface.

