#ifndef SYSCALL_H
#define SYSCALL_H

#define SYSCALL_MALLOC              0
#define SYSCALL_FREE                1
#define SYSCALL_VTHREAD_CREATE      2

#include <stdint.h>

void syscall(uint32_t *const argv);

#endif
