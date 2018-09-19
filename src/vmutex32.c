#include <vmutex32.h>

#include <stdint.h>

#define VMUTEX32_STATE_FREE     0u

void vmutex32_init(vmutex32_t *const mutex_ptr) {
    *mutex_ptr = VMUTEX32_STATE_FREE;
}
uint32_t const vmutex32_lock(vmutex32_t *const mutex_ptr, uint32_t const value) {
    static uint32_t mstatus_ie;
    static uint32_t mutex;

    __asm__ volatile ("csrrci %0, mstatus, 0x8" : "=r"(mstatus_ie) :: "memory");
    mstatus_ie &= 0x8;

    mutex = *mutex_ptr;
    if (mutex == VMUTEX32_STATE_FREE) {
        *mutex_ptr = value;
    }

    __asm__ volatile ("csrs mstatus, %0" :: "r"(mstatus_ie) : "memory");

    return mutex != VMUTEX32_STATE_FREE;
}
void vmutex32_wait_for_lock(vmutex32_t *const mutex_ptr, uint32_t const value) {
    while (vmutex32_lock(mutex_ptr, value));
}
void vmutex32_unlock(vmutex32_t *const mutex_ptr) {
    *mutex_ptr = VMUTEX32_STATE_FREE;
}
