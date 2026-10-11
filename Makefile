# The Shattered Crown - GBA build (devkitARM, no external libraries)
TARGET  := shattered_crown
DEVKITPRO ?= /opt/devkitpro
DEVKITARM ?= $(DEVKITPRO)/devkitARM
CC      := $(DEVKITARM)/bin/arm-none-eabi-gcc
OBJCOPY := $(DEVKITARM)/bin/arm-none-eabi-objcopy
GBAFIX  := $(DEVKITPRO)/tools/bin/gbafix

ARCH    := -mthumb -mthumb-interwork -mcpu=arm7tdmi -mtune=arm7tdmi
CFLAGS  := $(ARCH) -O2 -std=gnu11 -Wall -Wextra -fno-strict-aliasing
LDFLAGS := $(ARCH) -specs=gba.specs

SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=.o)

$(TARGET).gba: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@
	$(GBAFIX) $@ -tSHATTERCROWN -cSCRN -m01

$(TARGET).elf: $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

%.o: %.c *.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(TARGET).elf $(TARGET).gba

.PHONY: clean
