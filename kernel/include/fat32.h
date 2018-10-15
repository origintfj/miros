#ifndef FAT32_H
#define FAT32_H

#include <stdint.h>
#include <stddef.h>

#include <sd.h> // TODO - remove later when general file handle is available

#include <fat32_export.h>

// file system control
fat32_t *const fat32_mount(sd_context_t *const sd_context);
// file system navigation
int const fat32_dir_get(fat32_t *const fat32, fat32_entry_t *const dir_entry, char const path[]);
int const fat32_get_entry(fat32_t *const fat32, fat32_entry_t *const fat32_entry);
// file io
fat32_file_t *const fat32_open(fat32_t *const fat32, char const path[]);
int const fat32_close(fat32_file_t *const fat32_file_handle);
int const fat32_seek(fat32_file_t *stream, int long const offset, int const origin_id);
int long const fat32_tell(fat32_file_t *const stream);
size_t const fat32_read(void *const buffer, size_t const size, size_t const count, fat32_file_t *stream);

#endif
