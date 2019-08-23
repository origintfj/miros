#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

#define WRITE_REG(A, D)     *((uint32_t volatile *const)(A)) = (D)
#define READ_REG(A)         *((uint32_t volatile const *const)(A))

#endif
