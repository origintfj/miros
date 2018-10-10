#ifndef FAT32_H
#define FAT32_H

#include <stdint.h>
#include <stddef.h>

#define FAT32_ENTRY_ATTRIB_DIR      0x10
#define FAT32_MAX_PATH_LENGTH       256

#define FAT32_SEEK_SET              0
#define FAT32_SEEK_CUR              1
#define FAT32_SEEK_END              2

typedef struct fat32 fat32_t;

typedef struct {
    uint32_t dir_first_cluster;
    uint32_t entry_number;

    char short_name[13];
    uint8_t  attribute;
    uint32_t first_cluster;
    uint32_t file_szb;
} fat32_entry_t;

typedef struct fat32_file fat32_file_t;

// file system control
fat32_t *const fat32_mount(void *const fat32_img);
// file system navigation
int const fat32_dir_set(fat32_t const *const fat32, fat32_entry_t *const dir_entry, char const path[]);
int const fat32_get_entry(fat32_t const *const fat32, fat32_entry_t *const fat32_entry);
// file io
fat32_file_t *const fat32_open(fat32_t *const fat32, char const path[]);
int const fat32_seek(fat32_file_t *stream, int long const offset, int const origin_id);
int long const fat32_tell(fat32_file_t *const stream);
size_t const fat32_read(void *const buffer, size_t const size, size_t const count, fat32_file_t *stream);
int const fat32_close(fat32_file_t *const fat32_file_handle);

#endif
