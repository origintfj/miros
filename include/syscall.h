#ifndef SYSCALL_H
#define SYSCALL_H

#define SYSCALL_TIME_GET_UP_TIME_US     0
#define SYSCALL_VMEM32_MALLOC           1
#define SYSCALL_VMEM32_FREE             2
#define SYSCALL_VTHREAD_GETALL          3
#define SYSCALL_VTHREAD_CREATE          4

#include <stdint.h>

void syscall(uint32_t *const argv);

#endif
