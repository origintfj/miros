#ifndef VMEM32_H
#define VMEM32_H

#include <stddef.h>
#include <stdint.h>

void vmem32_dump_table(void);
void vmem32_init(uint8_t *const first_byte, uint8_t *const last_byte);
unsigned const vmem32_available(void);
void *const vmem32_alloc(size_t const n_bytes);
void vmem32_free(void *const ptr);

#endif
