# NUCLEOL476RG Assembly program for blinking with USER LED and Debugging with GDB

NucleoL476RG is a development board from STM.
It has STM32L476RGT6 MCU.
[Reference manual](https://www.st.com/resource/en/reference_manual/rm0351-stm32l47xxx-stm32l48xxx-stm32l49xxx-and-stm32l4axxx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)

In this post I will describe how to write simple one file assembly program
that will make the board blink with it's onboard LED2 and then I will show
how to debug it with GDB.
[Full code on GitHub]()

This post will closely follow my [previous post for BluePill](https://kalleva.bearblog.dev/note-007-blinking-with-led-on-blue-pill-with-assembly/).

## 1. Setting linker file

STM32L476RGT6 1024K of Flash memory and 128K of SRAM.
Smallest linker file should have variable holding the address of the start
of the stack (0x20000000 + 128K of RAM) and also section that descrbe memories avaliable on it.
I've put this information in file called ```STM32L476RGT6.ld```.

## 2. Writing assembly program

```core.s``` contains code of the program.
I went into more details on sections of this file in [Running simple assembly on STM32L071CBT6](https://kalleva.bearblog.dev/running-simple-assembly-on-stm32l071cbt6/)

1. First three lines contain some general info about MCU.

```asm
.syntax unified
.cpu cortex-m4
.thumb
```

After that there are declared some constants which should be looked up in the
reference manual.

```asm
.equ RCC_BASE_ADDRESS, 0x40021000
.equ RCC_AHB2ENR_ADDRESS_OFFSET, 0x4C
.equ RCC_AHB2ENR_GPIOA_EN,0x01
.equ GPIOA_BASE_ADDRESS, 0x48000000
.equ GPIOA_MODER_ADDRESS_OFFSET, 0x00
.equ GPIO_PIN5_MODE_OFFSET, 10 
.equ GPIO_MODER_PIN5_MASK, 0x03 << GPIO_PIN5_MODE_OFFSET
.equ GPIO_OUTPUT_MODE, 0x01
.equ GPIO_MODER_PIN5_OUTPUT, 0x01 << GPIO_PIN5_MODE_OFFSET 
.equ GPIOA_OTYPER_ADDRESS_OFFSET, 0x04
.equ GPIOA_OSPEEDR_ADDRESS_OFFSET, 0x08
.equ GPIOA_BSRR_ADDRESS_OFFSET, 0x18
.equ GPIOA_BSRR_PIN5_SET_MASK, 1 << 5 
.equ GPIOA_BSRR_PIN5_RESET_MASK, 1 << 21
.equ LOOP_END, 1000000
```

2. Next there is simplest vector table with the entry a reset_handler.
That reset_handler will contain all our code.

```asm
.type vector_table, %object
vector_table:
    .word _estack
    .word reset_handler
.size .type reset_handler, %function

reset_handler:
vector_table, .-vector_table
```

3. Inside reset_handler the first instructions are placing adress of the stack
to the stack_pointer.

```asm
LDR  r0, =_estack
MOV  sp, r0
```

4. User LED2 on Nucleo64 board is usually PA5. 
So next section is responsible for enabling GPIOA clock by
setting corresponding bit in RCC_APB2ENR registe

```asm
LDR r3, =RCC_AHB2ENR_ADDRESS_OFFSET
LDR r1, =RCC_BASE_ADDRESS @ Load RCC registers base address
LDR r2, [r1, r3] @ Load RCC_AHB2ENR register configuration
ORR r2, r2, RCC_AHB2ENR_GPIOA_EN @ Set bit to enable GPIOA
STR r2, [r1, r3] @ Store modified RCC_APB2ENR register configuration
```

5. After that there is section to configure PA5 bit as output 
push-pull low-speed pin. 

```asm
LDR r3, =GPIOA_MODER_ADDRESS_OFFSET
LDR r1, =GPIOA_BASE_ADDRESS @ Load GPIOA registers base address
LDR r2, [r1, r3] @ Load GPIOA_MODER Register value in r2
LDR r4, =GPIO_MODER_PIN5_MASK @ Load GPIOA_MODER_PIN5_MASK so we can work with only PA5 pin configuration
BIC r2, r2, r4 @ Clear configuration for PA5
 
@ Configure pin as general purpose output mode
LDR r4, =GPIO_MODER_PIN5_OUTPUT @ Load value of PC5 pin configuration
ORR r2, r2, r4 @ Set this value in register
STR r2, [r1, r3] @ Store value back to the register
	
@ Leave GPIOA_OTYPER Register PIN5 configuration to reset value: Output push-pull
@ Leave GPIOA_OSPEEDR Register PIN5 configuration to reset value: Low speed 	
```

6. To turn LED2 ON and OFF PA5 should be turned HIGH and LOW.
On STM32L4 this can be done with BSRR register.
To turn pin HIGH the corresponding BS bit in the BSRR register should be set,
and to turn pin LOW the correspondig BR bit in the same register
also should bet set.
I store the configuration for PA5 pin in register ```r6```.
Initially it is set HIGH.
And inside of a ```main_loop``` value inside ```r6`` is stored as a
configureaton for the BSRR register. 
After that the bits inside ```r6``` for setting and resetting PA5 pin are
flipped with a help of XOR of it's current value with SET_MASK and RESET_MASK.

```asm
STR r6, [r1, r0] @ Store configuration to BSRR register

EOR r6, r2 @ Flip PIN5 Enable bit. 
EOR r6, r3 @ Flip PIN5 Disable bit
@ Because it started with Enable bit set and Disable bit reset
@ it will flip to opposite states, so Enable and Disable will
@ not be 1 at the same time
```

7. Next there is ```delay_loop``` which gives some delay so you can see how
LED changes it's state.

```asm
@ Loop million times for delay
  MOV r4, 0
  LDR r5, =LOOP_END
delay_loop:
 	ADD r4, r4, #1
 	CMP r4, r5
	BLT delay_loop
```

8. After ```delay_loop``` there is jump to the start of a ```main_loop```:

```asm
B main_loop @ Go to the next iteration of main loop
```

And finish of reset_handler declaration:

```asm
.size reset_handler, .-reset_handler
```

## 3. Building project

Project is built with the help of a [GNU Arm Embedded Toolchain](https://developer.arm.com/downloads/-/gnu-rm) and following [Makefile]().
It's configured for Debug, so there are some additional settings for it:

```make
ASFLAGS += -Og
ASFLAGS += -g3 -ggdb -gdwarf-2
```

```STM32L476RGBlink.elf``` is a target which is created with this Makefile. 
```mingw32-make.exe``` from [MinGW64](https://www.mingw-w64.org/downloads/) is used
to build a project on Windows. For convenience it's better to add MingGW folders
to the PATH Environment Variable.

## 4. Debugging project

Debugging of the project is done with the help of [st-util](https://github.com/stlink-org/stlink/releases) and 
```arm-none-eabi-gdb```.
On Windows you should place folders from ```stlink-1.7.0-x86_64-w64-mingw32.zip```
in the corresponding folders of ```MinGW64/x86_64-w64-mingw32```.

1. First connect your NUCLEO and start ```st-util``` on your shell.
If everything is matching, there should be a message indicating 
that ```st-util``` started successfully.

```text
INFO gdb-server.c: Listening at *:4242...
```

2. Start **gdb** with an **.elf** file generated by the build process.

```
arm-none-eabi-gdb.exe ./STM32L476RGBlink.elf
```

This should bring you inside **gdb** with this message:

```
Reading symbols from ./STM32L476RGBlink.elf...
```

3. To connect to NUCLEO to the running ```st-util``` give **gdb** this command:

```
target extended-remote :4242
```

If command was successfull there should be a message indicating that debugging
session has started:

```
Remote debugging using :4242
```

4. To load firmware to device send a ```load``` command to **gdb**.
Response should be indication that the firmware is written to device.

```
Loading section .text, size 0x6c lma 0x8000000
Start address 0x08000000, load size 108
Transfer rate: 376 bytes/sec, 108 bytes/write.
```

5. Let's put a breakpoint inside the ```main_loop``` on the line 58,
where PA5 pin state configuration is stored to the BSSR register.
This is done with ```b 58``` command to **gdb**.

To list breakpoints that are placed in the programm type ```i b```.

6. Program is not started to start it out type ```s``` two times this will
make execute first instruction of the ```reset_handler```,
which loads stack adress to the ```r0``` register.
To see result of this command type ```i r``` to inspect the registers
and observe that ```r0``` register is indeed contains start of the stack.

```r0             0x20020000          537001984```

7. ```s``` **gdb** command lets step through the command one instruction
at the time. To continue execution until the breakpoint ```c``` command
can be used. After hitting breakpoint execution can be continued with ```c```
and LED2 will switch it's state on each iteration of the loop.
```r6``` register will hold either ```0x200000``` configuration
to turn OFF the LED, either 0x20 to turn in ON.
