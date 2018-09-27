#include <syscall.h>

#include <vmem32.h>

void syscall(uint32_t *const argv) {
    if (argv[0] == SYSCALL_MALLOC) {
        argv[1] = (uint32_t const)vmem32_alloc((size_t const)(argv[1]));
    } else if (argv[0] == SYSCALL_FREE) {
        vmem32_free((void *const)(argv[1]));
    }
}
