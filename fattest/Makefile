all: fat32fs
	xxd -i fat32fs.img fat32fs.h
	gcc -I../include main.c ../src/fat32.c -o out
	./out

fat32fs:
	dd if=/dev/zero of=fat32fs.img bs=1K count=100
	mkfs.vfat -v -F32 fat32fs.img
	mkdir -p mnt
	sudo mount -t vfat -o uid=$$(whoami) fat32fs.img mnt/
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
