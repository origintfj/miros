SHELL := /bin/bash

DEVICE ?= /dev/ttyUSB0

.PHONY: all
all: clean kernel upload
	#pushd apps ; make ; popd
	#pushd fat32 ; make ; popd
	#cp fat32/fat32fs.x .

.PHONY: kernel
kernel:
	pushd kernel ; make ; popd
	cp kernel/miros.hex .

.PHONY: upload
upload:
	sudo cat miros.hex > $(DEVICE)

.PHONY: clean
clean:
	pushd kernel ; make clean ; popd
	pushd apps ; make clean ; popd
	#pushd fat32 ; make clean ; popd
	rm -f *.x *.hex
