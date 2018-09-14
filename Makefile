TOOLCHAIN_DIR := /opt/rv32i/

CC      := $(TOOLCHAIN_DIR)/bin/riscv32-unknown-elf-gcc
AR      := $(TOOLCHAIN_DIR)/bin/riscv32-unknown-elf-gcc-ar
OBJCOPY := $(TOOLCHAIN_DIR)/bin/riscv32-unknown-elf-objcopy
OBJDUMP := $(TOOLCHAIN_DIR)/bin/riscv32-unknown-elf-objdump

LD_OPTIONS := -L$(TOOLCHAIN_DIR)/lib/gcc/riscv32-unknown-elf/7.2.0/ -T miros.ld
CC_OPTIONS := -I./include -nostdlib -fPIC -O3
SRC_FILES := 				\
	src/entry.S				\
	src/uart.c				\
	src/boot.c

build:
	$(CC) $(LD_OPTIONS) $(CC_OPTIONS) $(SRC_FILES) -lgcc -o miros.out
	$(OBJCOPY) -O ihex miros.out miros.hex
	$(OBJDUMP) -D miros.out > miros.dis
	#$(AR) rcs libsos32.a src/system.c

clean:
	rm *.o *.dis *.a
