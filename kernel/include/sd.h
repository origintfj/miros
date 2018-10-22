#ifndef SD_H
#define SD_H

#include <stdint.h>
#include <stddef.h>

typedef struct sd_context sd_context_t;

#define SD_SEEK_SET     0
#define SD_SEEK_CUR     1
#define SD_SEEK_END     2

sd_context_t *const sd_context_create(uint32_t const spim_base_addr, uint32_t const clk_freq_hz);
int const sd_context_destroy(sd_context_t *const sd_context);
int const sd_init(sd_context_t *const sd_context);
int const sd_seek(sd_context_t *const sd_context, uint64_t const offset, int const origin);
uint64_t const sd_tell(sd_context_t *const sd_context);
size_t const sd_read(void *const buffer, size_t const size,
                     size_t const count, sd_context_t *const sd_context);

#endif
