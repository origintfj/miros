#include <syscall.h>

#include <vmem32.h>
#include <vthread32.h>

uint64_t const time_us(void); // TODO - move to time module

void syscall(uint32_t *const argv) {
    if (argv[0] == SYSCALL_TIME_GET_UP_TIME_US) {
        uint64_t const uptime_us = time_us();
        argv[1] = (uint32_t const)(uptime_us >>  0);
        argv[2] = (uint32_t const)(uptime_us >> 32);
    } else if (argv[0] == SYSCALL_VMEM32_AVAILABLE) {
        argv[1] = (uint32_t const)vmem32_available();
    } else if (argv[0] == SYSCALL_VMEM32_MALLOC) {
        argv[1] = (uint32_t const)vmem32_alloc((size_t const)(argv[1]));
    } else if (argv[0] == SYSCALL_VMEM32_FREE) {
        vmem32_free((void *const)(argv[1]));
    } else if (argv[0] == SYSCALL_VTHREAD_GETALL) {
        argv[1] = (uint32_t const)vthread32_get_all((thread_handle_t **const)(argv[1]));
    } else if (argv[0] == SYSCALL_VTHREAD_CREATE) {
        argv[1] = (uint32_t const)vthread32_create((void *const(*)(void *const))argv[1],
                                                   (void *const)argv[2], 1024u, 0x0080);
    }
}
