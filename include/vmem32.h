#ifndef VMEM32_H
#define VMEM32_H

#include <stddef.h>
#include <stdint.h>

#define VMEM32_NULL     (void *const)0

void vmem32_dump_table(void);
void vmem32_init(uint8_t *const first_byte, uint8_t *const last_byte);
void *const vmem32_alloc(size_t const n_bytes);
void vmem32_free(void *const ptr);

#endif
