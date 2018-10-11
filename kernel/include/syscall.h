#ifndef SYSCALL_H
#define SYSCALL_H

#define SYSCALL_TIME_GET_UP_TIME_US     0
#define SYSCALL_VMEM32_AVAILABLE        1
#define SYSCALL_VMEM32_MALLOC           2
#define SYSCALL_VMEM32_FREE             3
#define SYSCALL_VTHREAD_GETALL          4
#define SYSCALL_VTHREAD_CREATE          5
#define SYSCALL_FAT32_MOUNT             6
#define SYSCALL_FAT32_UMOUNT            7  // TODO - implement
#define SYSCALL_FAT32_DIR_SET           8
#define SYSCALL_FAT32_GET_ENTRY         9
#define SYSCALL_FAT32_OPEN              10
#define SYSCALL_FAT32_CLOSE             11
#define SYSCALL_FAT32_SEEK              12
#define SYSCALL_FAT32_TELL              13
#define SYSCALL_FAT32_READ              14

#include <stdint.h>

void syscall(uint32_t *const argv);

#endif
