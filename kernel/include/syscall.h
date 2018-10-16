#ifndef SYSCALL_H
#define SYSCALL_H

#define SYSCALL_TIME_GET_UP_TIME_US     100
#define SYSCALL_VMEM32_AVAILABLE        201
#define SYSCALL_VMEM32_MALLOC           202
#define SYSCALL_VMEM32_FREE             203
#define SYSCALL_VTHREAD_FINISHED        300
#define SYSCALL_VTHREAD_GETALL          304
#define SYSCALL_VTHREAD_CREATE          305
#define SYSCALL_FAT32_MOUNT             406
#define SYSCALL_FAT32_UMOUNT            407  // TODO - implement
#define SYSCALL_FAT32_DIR_SET           408
#define SYSCALL_FAT32_GET_ENTRY         409
#define SYSCALL_FAT32_OPEN              410
#define SYSCALL_FAT32_CLOSE             411
#define SYSCALL_FAT32_SEEK              412
#define SYSCALL_FAT32_TELL              413
#define SYSCALL_FAT32_READ              414
#define SYSCALL_PROC_CREATE             500

#include <stdint.h>

void syscall(uint32_t *const argv);

#endif
