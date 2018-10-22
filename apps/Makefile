TOOLCHAIN_DIR := /opt/rv32i/

CC      := $(TOOLCHAIN_DIR)/bin/riscv32-unknown-elf-gcc
LD      := $(TOOLCHAIN_DIR)/bin/riscv32-unknown-elf-ld
AR      := $(TOOLCHAIN_DIR)/bin/riscv32-unknown-elf-gcc-ar
OBJCOPY := $(TOOLCHAIN_DIR)/bin/riscv32-unknown-elf-objcopy
OBJDUMP := $(TOOLCHAIN_DIR)/bin/riscv32-unknown-elf-objdump

LD_OPTIONS := -L$(TOOLCHAIN_DIR)/lib/gcc/riscv32-unknown-elf/7.2.0/ -T app.ld --no-relax
CC_OPTIONS := -march=rv32ic -I../kernel/include -nostdlib -fPIC -ffreestanding -O3
SRC_FILES := 				\
	entry.S					\
	../kernel/src/uart.c	\
	../kernel/src/miros.c	\
	../kernel/src/vstring.c	\
	$(APP).c
OBJ_FILES := 				\
	entry.o					\
	uart.o					\
	miros.o					\
	vstring.o				\
	$(APP).o

build:
	$(CC) $(SRC_FILES) $(CC_OPTIONS) -c
	$(LD) $(OBJ_FILES) $(LD_OPTIONS) -lgcc -o $(APP).out
	$(OBJCOPY) -O ihex $(APP).out $(APP).hex
	$(OBJCOPY) -O binary $(APP).out $(APP)
	$(OBJDUMP) -D $(APP).out > $(APP).dis
	#$(AR) rcs libsos32.a src/system.c
	#ll fattest/fat32fs.img

clean:
	rm -f *.o *.dis *.a *.out *.hex
