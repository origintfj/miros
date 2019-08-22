TOOLCHAIN_DIR    ?= /opt/rv32imc/
TOOLCHAIN_PREFIX ?= riscv32-unknown-elf-

CC      := $(TOOLCHAIN_DIR)/bin/$(TOOLCHAIN_PREFIX)gcc
LD      := $(TOOLCHAIN_DIR)/bin/$(TOOLCHAIN_PREFIX)ld
AR      := $(TOOLCHAIN_DIR)/bin/$(TOOLCHAIN_PREFIX)gcc-ar
OBJCOPY := $(TOOLCHAIN_DIR)/bin/$(TOOLCHAIN_PREFIX)objcopy
OBJDUMP := $(TOOLCHAIN_DIR)/bin/$(TOOLCHAIN_PREFIX)objdump

LD_OPTIONS := -L$(TOOLCHAIN_DIR)/lib/gcc/riscv32-unknown-elf/7.2.0/ -T miros.ld --no-relax
CC_OPTIONS := -march=rv32ic -I./include -nostdlib -fPIC -ffreestanding -O3

SRC_CFILES = $(wildcard src/*.c)
SRC_SFILES = $(wildcard src/*.S)
SRC_FILES  = $(SRC_CFILES) $(SRC_SFILES)

OBJ_FILES  = $(patsubst src/%.c,%.o,$(SRC_CFILES))
OBJ_FILES += $(patsubst src/%.S,%.o,$(SRC_SFILES))

.PHONY: kernel
kernel:
	$(CC) $(SRC_FILES) $(CC_OPTIONS) -c
	$(LD) $(OBJ_FILES) $(LD_OPTIONS) -lgcc -o miros.out
	$(OBJCOPY) -O ihex miros.out miros.hex
	$(OBJCOPY) -O binary miros.out miros.bin
	$(OBJDUMP) -D miros.out > miros.dis
	#$(AR) rcs libsos32.a src/system.c
	#ll fattest/fat32fs.img

.PHONY: clean
clean:
	rm -f *.o *.dis *.a *.out *.hex *.bin
