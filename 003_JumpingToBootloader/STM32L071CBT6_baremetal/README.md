# Running simple assembly on STM32L071CBT6

This post is heavily indebted to the wonderful series of blog posts on https://vivonomicon.com/2018/04/02/bare-metal-stm32-programming-part-1-hello-arm/ definitely go and check it out I got started with baremetal stm32 from him.

My post is adapted version for my workflow on Windows.

## Prerequisites

1. Install [STM32CubeProgrammer](https://www.st.com/en/development-tools/stm32cubeprog.html).

2. Install [GNU Arm Embedded Toolchain](https://developer.arm.com/downloads/-/gnu-rm)
    and add ```GNU Arm Embedded Toolchain\10 2021.10\bin`` to Windows Environment Variables PATH so you can call toolchain modules
    without prefixing path. Makefile I use is counting on that.

3. Install MinGW and add it to Windows Environment Variables PATH so you can call mingw32-make.exe

4. Get debug probe for flashing and debugging your MCU via SWD. 

## Walkthrough

1. Create minimal linker script **STM32L071CBT6.ld**.
We need to  tell compiler linker how much FLASH for storing program code and how much RAM chip has.
And we need to set the value of the end of the stack in _estack (0x20000000 + 20K).

```ld
_estack = 0x20005000;

MEMORY
{
    FLASH ( rx )      : ORIGIN = 0x08000000, LENGTH = 192K
    RAM ( rxw )       : ORIGIN = 0x20000000, LENGTH = 20K
}
```

2. Create file assembly file core.s

First 4 lines describe cpu and syntax for compiler.

```asm
.syntax unified
.cpu cortex-m0plus
.fpu softvfp
.thumb
```

Next we put vector table with two entries. First points to the end of stack and next to the reset_handler function, that will be
executed after MCU starts.

```asm
.type vector_table, %object
vector_table:
    .word _estack
    .word reset_handler
.size vector_table, .-vector_table
```

And lastly we need to define reset_handler:

```asm
.type reset_handler, %function
reset_handler:
  LDR  r0, =_estack
  MOV  sp, r0

  LDR  r7, =0xDEADBEEF
  MOVS r0, #0
  main_loop:
    ADDS r0, r0, #1
    B    main_loop
.size reset_handler, .-reset_handler
```
It sets ```sp``` (stack pointer) to the end of the stack.
Then loads value that we will easily recognize to the register ```r7``` and puts ```0``` in register ```r0```.
And the defines label ```main_loop```, after that it increments value in r0 and branches back to ```main_loop```.

And this is it. Now we need to compile it and watch it run live.

3. Compiling
I took Makefile from https://github.com/WRansohoff/STM32F0_minimal/blob/master/Makefile and a little adapted it.

```Makefile
# Makefile for compiling ARM Cortex-M0 assembly projects.
TARGET = 001

# Define the linker script location and chip architecture.
LD_SCRIPT = STM32L071CBT6.ld
MCU_SPEC = cortex-m0plus

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

When you run mingw32-make.exe you should get 001.elf file.

4. Lets inspect what we got:

Run ```arm-none-eabi-nm.exe 001.elf``` and you should get this:

```
20005000 A _estack
08000010 t main_loop
08000008 t reset_handler
08000000 t vector_table
```

```vector_table``` with its two entries placed in the very beginning of memory. After that in memory placed ```reset_handler``` and ```main_loop```.

Run ```arm-none-eabi-objdump.exe -d .\001.elf``` to dissasemble elf file.

```disassembly
.\001.elf:     file format elf32-littlearm


Disassembly of section .text:

08000000 <vector_table>:
 8000000:       00 50 00 20 09 00 00 08                             .P. ....

08000008 <reset_handler>:
 8000008:       4802            ldr     r0, [pc, #8]    ; (8000014 <main_loop+0x4>)
 800000a:       4685            mov     sp, r0
 800000c:       4f02            ldr     r7, [pc, #8]    ; (8000018 <main_loop+0x8>)
 800000e:       2000            movs    r0, #0

08000010 <main_loop>:
 8000010:       3001            adds    r0, #1
 8000012:       e7fd            b.n     8000010 <main_loop>
 8000014:       20005000        .word   0x20005000
 8000018:       deadbeef        .word   0xdeadbeef
```

First you can see that it indeed use Thumb mode for because instructions are 16-bit wide.
Next you can see that first we load ```_eastack``` ```0x20005000``` to the ```sp```.
Then we put ```0xDEADBEEF``` to ```r7``` and ```0``` to ```r0``` and start incrementing ```r0``` in ```main_loop```.

5. Now it's time to load 001.elf to the MCU and make sure that program runs as expected.
Start STM32CubeProgrammer and connect to device via SWD. Go it's **Erasing&Programming** page and load 001.elf to the board.
In newer version of STM32CubeProgrammer there is a page **MCU core** that lets you debug your code on MCU.
So go **MCU core** page where we go through code step by step.

- Hit **Software reset** button and **Read Core Reg** button after that. You get state of your registers at the start of a program.
![001.jpg](./images/001.jpg)

- Hit **Step** button. As you can see ```_estack``` is loaded to ```r0``` and ```pc``` Program Counter advances by 2 bytes to the next instruction.
![002.jpg](./images/002.jpg)

- Hit **Step** button. ```sp``` is updated with ```_estack``` value from ```r0```.
![003.jpg](./images/003.jpg)

- Hit **Step** button. ```0xDEADBEEF``` is loaded to ```r7```.
![004.jpg](./images/004.jpg)

- Hit **Step** button. ```0``` is loaded to ```r0```.
![005.jpg](./images/005.jpg)

- Hit **Step** button. ```0``` in  ```r0``` is incremented and becomes ```1```.
![006.jpg](./images/006.jpg)

- Hit **Step** button. Brunch to ```main_loop``` label is performed. ```pc``` goes from ```0x08000012``` to ```0x08000010```
![007.jpg](./images/007.jpg)

- Hit **Step** button. ```1``` in  ```r0``` is incremented and becomes ```2```.
![008.jpg](./images/008.jpg)
