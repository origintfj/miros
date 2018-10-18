#ifndef MIROS_H
#define MIROS_H

#include <stddef.h>
#include <stdint.h>

#include <vstring.h>

#include <sd.h>
#include <fat32_export.h>

// timer functions
uint64_t const get_up_time_us(void);
// memory allocation functions
uint32_t const mavailable(void);
void *const malloc(size_t const szb);
void free(void *const buffer);
// thread functions
unsigned const vthread_get_all(uint32_t **const list);
uint64_t const vthread_create(void *const(*thread)(void *const), void *const arg);
// file system functions
fat32_t *const fs_mount(sd_context_t *const sd_context);
int const fs_dir_get(fat32_entry_t *const dir_entry, char const path[]);
int const fs_get_entry(fat32_entry_t *const fat32_entry);
fat32_file_t *const fopen(char const path[]);
int const fclose(fat32_file_t *const fat32_file_handle);
int const fseek(fat32_file_t *stream, int long const offset, int const origin_id);
int long const ftell(fat32_file_t *const stream);
size_t const fread(void *const buffer, size_t const size, size_t const count, fat32_file_t *stream);
// process functions
uint32_t const proc_start(char const *const path, int const argc, char const *const *const argv);

#endif
