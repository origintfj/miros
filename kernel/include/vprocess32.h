#ifndef VPROCESS32_H
#define VPROCESS32_H

#include <stdint.h>
#include <fat32.h>
#include <vthread32.h>

thread_id_t const vprocess32_start(fat32_t *const fat32, char const *const path, unsigned const stack_szw,
                                   int const argc, char const *const *const argv);

#endif
