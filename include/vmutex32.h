#ifndef VMUTEX32_H
#define VMUTEX32_H

#include <stdint.h>

#define VMUTEX32_STATE_FREE     0u
#define VMUTEX32_STATE_LOCKED   1u

typedef volatile uint32_t vmutex32_t;

void vmutex32_init(vmutex32_t *const mutex_ptr);
uint32_t const vmutex32_lock(vmutex32_t *const mutex_ptr, uint32_t const value);
void vmutex32_wait_for_lock(vmutex32_t *const mutex_ptr, uint32_t const value);
void vmutex32_unlock(vmutex32_t *const mutex_ptr);

#endif
