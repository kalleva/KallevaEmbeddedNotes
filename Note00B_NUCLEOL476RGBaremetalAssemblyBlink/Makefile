# Makefile for compiling ARM Cortex-M4 assembly projects.
STM32L476RGBlink

# Define the linker script location and chip architecture.
LD_SCRIPT = STM32L476RGT6.ld
MCU_SPEC = cortex-m4
FPU = -mfpu=fpv4-sp-d16
FLOAT-ABI = -mfloat-abi=hard

# Toolchain definitions (ARM bare metal defaults)
CC = arm-none-eabi-gcc
AS = arm-none-eabi-as
LD = arm-none-eabi-ld
OC = arm-none-eabi-objcopy
OD = arm-none-eabi-objdump
OS = arm-none-eabi-size

# Assembly directives.
ASFLAGS += -c
ASFLAGS += -Og
ASFLAGS += -mcpu=$(MCU_SPEC)
ASFLAGS += $(FPU)
ASFLAGS += $(FLOAT-ABI)
ASFLAGS += -mthumb
ASFLAGS += -Wall
# (Set error messages to appear on a single line.)
ASFLAGS += -fmessage-length=0
ASFLAGS += -g3 -ggdb -gdwarf-2
ASFLAGS += -fdata-sections
ASFLAGS += -ffunction-sections
ASFLAGS += -Wextra
ASFLAGS += -Wpedantic
ASFLAGS += -fstack-usage

# Linker directives.
LSCRIPT = ./$(LD_SCRIPT)
LFLAGS += -mcpu=$(MCU_SPEC)
LFLAGS += $(FPU)
LFLAGS += $(FLOAT-ABI)
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
