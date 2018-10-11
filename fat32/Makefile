SIZE_KB ?= 100

all: fat32fs

fat32fs:
	dd if=/dev/zero of=fat32fs.img bs=1K count=$(SIZE_KB)
	mkfs.vfat -v -F32 fat32fs.img
	mkdir -p mnt
	sudo mount -t vfat -o uid=$$(whoami) fat32fs.img mnt/
	mkdir -p mnt/bin
	cp ../apps/*.bin mnt/bin/
	cp ../apps/*.c mnt/bin/
	mkdir -p mnt/sys/kernel
	mkdir -p mnt/root/sub
	echo "test1" > mnt/test1.txt
	echo "test2" > mnt/root/test2.txt
	echo "test3" > mnt/root/sub/test3.txt
	sudo umount mnt/
	xxd -p fat32fs.img > fat32fs.x

mount:
	sudo mount -t vfat -o uid=$$(whoami) fat32fs.img mnt/

umount:
	sudo umount mnt/
