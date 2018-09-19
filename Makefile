TOOLCHAIN_DIR := /opt/rv32i/

CC      := $(TOOLCHAIN_DIR)/bin/riscv32-unknown-elf-gcc
LD      := $(TOOLCHAIN_DIR)/bin/riscv32-unknown-elf-ld
AR      := $(TOOLCHAIN_DIR)/bin/riscv32-unknown-elf-gcc-ar
OBJCOPY := $(TOOLCHAIN_DIR)/bin/riscv32-unknown-elf-objcopy
OBJDUMP := $(TOOLCHAIN_DIR)/bin/riscv32-unknown-elf-objdump

LD_OPTIONS := -L$(TOOLCHAIN_DIR)/lib/gcc/riscv32-unknown-elf/7.2.0/ -T miros.ld --no-relax
CC_OPTIONS := -I./include -nostdlib -fPIC -ffreestanding -O3
SRC_FILES := 				\
	src/entry.S				\
	src/vmutex32.c			\
	src/vthread32_asm.S		\
	src/vthread32.c			\
	src/vmem32.c			\
	src/uart.c				\
	src/boot.c
OBJ_FILES := 				\
	entry.o					\
	vmutex32.o				\
	vthread32_asm.o			\
	vthread32.o				\
	vmem32.o				\
	uart.o					\
	boot.o

build:
	$(CC) $(SRC_FILES) $(CC_OPTIONS) -c
	$(LD) $(OBJ_FILES) $(LD_OPTIONS) -lgcc -o miros.out
	$(OBJCOPY) -O ihex miros.out miros.hex
	$(OBJDUMP) -D miros.out > miros.dis
	#$(AR) rcs libsos32.a src/system.c

clean:
	rm -f *.o *.dis *.a
