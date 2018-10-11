#ifndef SPIM_H
#define SPIM_H

#include <stdint.h>

int const spim_init(uint32_t const dev_base_addr, unsigned const cphb, int const cpol, int const cpha);
int const spim_set_cphb(uint32_t const dev_base_addr, unsigned const cphb);
void spim_cs_assert(uint32_t const dev_base_addr);
void spim_cs_deassert(uint32_t const dev_base_addr);
void spim_block_exchange(uint32_t const dev_base_addr, uint8_t *const dest_buffer,
                         uint8_t const *const src_buffer, unsigned const count);
void spim_block_read(uint32_t const dev_base_addr,
                     uint8_t *const dest_buffer, unsigned const count);
void spim_block_write(uint32_t const dev_base_addr,
                      uint8_t const *const src_buffer, unsigned const count);
uint8_t const spim_byte_exchange(uint32_t const dev_base_addr, uint8_t const tx_data);

#endif
