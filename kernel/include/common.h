#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

inline void write_reg(uint32_t const addr, uint32_t const data) {
    *((uint32_t volatile *const)addr) = data;
}

inline uint32_t volatile const read_reg(uint32_t const addr) {
    return *((uint32_t volatile const *const)addr);
}

#endif
