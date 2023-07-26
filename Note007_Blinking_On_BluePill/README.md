# Blinking With LED On Blue Pill with Assembly

In this post I will show you how to blink with user led on blue pill board only using assembly.
This post uses template from one of my [earlier posts](https://kalleva.bearblog.dev/running-simple-assembly-on-stm32l071cbt6/).
You can check it out because I will skip over some things that I described in that post.

Blue pill board uses STM32F103C8 MCU. To start programming it in assembly you need to get [RM0008 Reference manual](https://www.st.com/resource/en/reference_manual/cd00171190-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-arm-based-32-bit-mcus-stmicroelectronics.pdf).
And also [this overview](https://developer.arm.com/documentation/dui0473/m/arm-and-thumb-instructions/) of ARM assembly instructions will come in handy.

This is [code for this post](https://github.com/kalleva/KallevaEmbeddedNotes/tree/master/Note007_Blinking_On_BluePill).

## Initial template for the project

1. Assembly file ```core.s``` will contain code for our program.
Start out by creating linker file ```STM32F103C8.ld``` with minimal description of memory our MCU has and stack base address which you can get from datasheet.

```text
_estack = 0x20005000;

MEMORY
{
    FLASH ( rx )      : ORIGIN = 0x08000000, LENGTH = 128K
    RAM ( rxw )       : ORIGIN = 0x20000000, LENGTH = 20K
}
```

2. After that modify ```Makefile``` to reflect the fact that we are using cortex-m3 and point linker script to the one we created.

```Makefile
# Makefile for compiling ARM Cortex-M3 assembly projects.
TARGET = STM32F103C8_Blink

# Define the linker script location and chip architecture.
LD_SCRIPT = STM32F103C8.ld
MCU_SPEC = cortex-m3

# Toolchain definitions (ARM bare metal defaults)
CC = arm-none-eabi-gcc
AS = arm-none-eabi-as
LD = arm-none-eabi-ld
OC = arm-none-eabi-objcopy
OD = arm-none-eabi-objdump
OS = arm-none-eabi-size

# Assembly directives.
ASFLAGS += -c
ASFLAGS += -O0
ASFLAGS += -mcpu=$(MCU_SPEC)
ASFLAGS += -mthumb
ASFLAGS += -Wall
# (Set error messages to appear on a single line.)
ASFLAGS += -fmessage-length=0

# Linker directives.
LSCRIPT = ./$(LD_SCRIPT)
LFLAGS += -mcpu=$(MCU_SPEC)
LFLAGS += -mthumb
LFLAGS += -Wall
LFLAGS += --specs=nosys.specs
LFLAGS += -nostdlib
LFLAGS += -lgcc
LFLAGS += -T$(LSCRIPT)

AS_SRC += core.s

OBJS =  $(AS_SRC:.s=.o)

.PHONY: all
all: $(TARGET).elf

%.o: %.s
	$(CC) -x assembler-with-cpp $(ASFLAGS) $< -o $@

$(TARGET).elf: $(OBJS)
	$(CC) $^ $(LFLAGS) -o $@

.PHONY: clean
clean:
	del $(OBJS)
	del $(TARGET).elf
```

3. Fill in ```core.s``` with initial code, so that our program will compile properly for our MCU and start executing after MCU reset.
I'm going into more details on this part in [this post](https://kalleva.bearblog.dev/running-simple-assembly-on-stm32l071cbt6/).

```asm
.syntax unified
.cpu cortex-m3
.fpu softvfp
.thumb

.type vector_table, %object
vector_table:
    .word _estack
    .word reset_handler
.size vector_table, .-vector_table

.type reset_handler, %function
reset_handler:
  LDR  r0, =_estack
  MOV  sp, r0
```

So at this point we are ready to start writing code for actually make our board blink with a LED.

## Blinking with LED

1. According to reference manual ```GPIOC``` connected to ```APB2``` bridge on ```AHB``` bus (Figure 1. System architecture (low-, medium-, XL-density devices).
So to enable ```GPIOC``` peripheral we need enable it in ```RCC_APB2ENR``` register.
You can do it in assembly by loading into ```r1``` register base address of ```RCC``` configuration registers.
You can find it in **Memory map** section of manual in **Table 3**.
```Reset and clock control RCC: 0x4002 1000```
We will set this value as a constant at the beginning of the ```core.s``` file.

```asm
.equ RCC_BASE_ADDRESS, 0x40021000
```

In the reset_handler function load ```RCC``` registers base address to ```r1```. After that load value of the ```RCC_APB2ENR``` configuration register to ```r2```. ```RCC_APB2ENR``` has offset ```0x18``` from the ```0x40021000``` ```RCC_BASE_ADDRESS```.
As you can see in ```8.3.7 APB2 peripheral clock enable register (RCC_APB2ENR)``` section of reference manual to enable ```GPIOC``` you need set fifth bit in that register. You can do it with ```ORR``` instruction.
And after that save that modified value back to memory, so it can take change.

```asm
LDR r1, =RCC_BASE_ADDRESS     @ Load RCC registers base address
LDR r2, [r1, #0x18]           @ Load RCC_APB2ENR register configuration
ORR r2, r2, #0x10             @ Set bit to enable GPIOC
STR r2, [r1, #0x18]           @ Store modified RCC_APB2ENR register configuration
```

2. Next you need to configure ```PC13``` pin as output push-pull.
To configure ```GPIOC``` registers we load ```GPIOC``` registers base address from **Memory map** section, and set them to ```.equ GPIOC_BASE_ADDRESS, 0x40011000``` constant.

To configure ```PC13``` (output mode slow speed with push-pull mode) pin as you can see in ```9.2.2 Port configuration register high (GPIOx_CRH) (x=A..G)``` you need to set 22 and 23 bits to ```00``` (output push-pull) and 20 and 21 to ```01``` (output mode slow speed).
And this ```GPIOC_CRH``` had offset of ```0x04``` from the ```GPIOC``` base register address.
To do this you need to load value from ```GPIOC_CRH``` register, clear bits that correspond to ```PC13``` configuration with ```BIC``` bit clear instruction. It works by clearing the bits in the second operand that are set in the third operand and saves the result in the first operand.
After that we just set configuration and store that configuration to memory.

```asm
LDR r1, =GPIOC_BASE_ADDRESS   @ Load GPIOC registers base address
LDR r2, [r1, #0x04]           @ Load GPIOC_CRH register configuration
LDR r3, =GPIOC_CRH_PIN13_MASK @ Load GPIOC_CRH_PIN13_MASK so we can work with only PC13 pin configuration
BIC r2, r2, r3                @ Clear configuration for PC13

@ Configure PC13 as MODE Output mode, max speed 10 MHz: 01
@ and CNF General purpose output push-pull: 00
@ Other pins left as reset value 4 (0b0100) - Floating input
LDR r3, =GPIOC_CRH_PIN13_OUTPUT_PUSH_PULL @ Load value of PC13 pin configuration
ORR r2, r2, r3                @ Set this value in register
STR r2, [r1, #0x04]
```

With this you have finished configuring ```PC13``` pin and can start control LED with value in the ```GPIOC_ODR``` register.

3. You can control if LED is ON or OFF by setting ```PC13``` high or low through writing ```1``` or ```0``` to ```GPIOC_ODR``` register.
So we start by loading mask with 13-th bit set to 1 to the ```r0``` register.
Register ```r2``` will hold value that will be set to ```GPIOC_ODR``` register in memory.

Now, to make program run continuously we need some kind of endless loop - ```main_loop```.
On each iteration of this loop you toggle the bit that corresponds to ```PC13``` in ```GPIOC_ODR```.
To toggle pin, I use ```EOR``` Exclusive OR instruction to XOR ```r2``` with ```PC13``` pin mask and store result back at ```r2```.
After that that XORed configuration for ```GPIOC_ODR``` register is stored to memory to take effect.

After that LED will change it's state, but next iteration of loop will happen very fast, and with it LED will change it's state again.
So we need to add some delay to be able to see LED blinking.

For this I introduces ```delay_loop``` which will simply increment value in ```r3``` register from ```0``` while it is less than  ```1 000 0000```. When value in ```r3``` reaches ```1 000 000``` stored in ```r4``` register execution breaks out of ```delay_loop```.
And next executed instruction is branch to the next iteration of the ```main_loop```.

```asm
  LDR r0, =GPIOC_ODR_PIN13_MASK
  MOV r2, #0                    @ r1 register will hold value which we will set on GPIOC_ODR register
main_loop:
  EOR r2, r2, r0                @ XOR GPIOC_ODR_PIN13_MASK with current value in r1. This will have effect of flipping the bit for PIN13
  STR r2, [r1, #0x0C]           @ Set ODR configuration

  @ Loop million times for delay
  MOV r3, 0
  LDR r4, =LOOP_END
delay_loop:
  ADD r3, r3, #1
  CMP r3, r4
  BLT delay_loop
  
  @ Go to the next iteration of main loop
  B main_loop
```

And that's it. Program will run indefinitely blinking approximately every second.

4. You can compile it with the help of make from MinGW and flash with [STM32CubeProgrammer](https://www.st.com/en/development-tools/stm32cubeprog.html). You can read more about how to compile, flash and debug execution of this firmware in [earlier post](https://kalleva.bearblog.dev/running-simple-assembly-on-stm32l071cbt6/).
