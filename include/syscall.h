#ifndef SYSCALL_H
#define SYSCALL_H

#define SYSCALL_TIME_GET_UP_TIME_US     0
#define SYSCALL_VMEM32_AVAILABLE        1
#define SYSCALL_VMEM32_MALLOC           2
#define SYSCALL_VMEM32_FREE             3
#define SYSCALL_VTHREAD_GETALL          4
#define SYSCALL_VTHREAD_CREATE          5

#include <stdint.h>

void syscall(uint32_t *const argv);

#endif
