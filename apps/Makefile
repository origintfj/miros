TOOLCHAIN_DIR := /opt/rv32i/

CC      := $(TOOLCHAIN_DIR)/bin/riscv32-unknown-elf-gcc
LD      := $(TOOLCHAIN_DIR)/bin/riscv32-unknown-elf-ld
AR      := $(TOOLCHAIN_DIR)/bin/riscv32-unknown-elf-gcc-ar
OBJCOPY := $(TOOLCHAIN_DIR)/bin/riscv32-unknown-elf-objcopy
OBJDUMP := $(TOOLCHAIN_DIR)/bin/riscv32-unknown-elf-objdump

LD_OPTIONS := -L$(TOOLCHAIN_DIR)/lib/gcc/riscv32-unknown-elf/7.2.0/ -T app.ld --no-relax
CC_OPTIONS := -I./include -nostdlib -fPIC -ffreestanding -O3
SRC_FILES := 				\
	test.c
OBJ_FILES := 				\
	test.o

build:
	$(CC) $(SRC_FILES) $(CC_OPTIONS) -c
	$(LD) $(OBJ_FILES) $(LD_OPTIONS) -lgcc -o test.out
	$(OBJCOPY) -O ihex test.out test.hex
	$(OBJCOPY) -O binary test.out test.bin
	$(OBJDUMP) -D test.out > test.dis
	#$(AR) rcs libsos32.a src/system.c
	#ll fattest/fat32fs.img

clean:
	rm -f *.o *.dis *.a *.out *.hex
