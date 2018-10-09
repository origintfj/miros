#ifndef FAT32_H
#define FAT32_H

#include <stdint.h>
#include <stddef.h>

#define FAT32_ENTRY_ATTRIB_DIR      0x10
#define FAT32_MAX_PATH_LENGTH       256

typedef struct fat32 {
    uint8_t *fs_img;
    //
    uint32_t sector_szb;
    uint32_t cluster_sz_sectors;
    uint32_t rsvd_sector_count;
    uint32_t fat_count;
    uint32_t fat_sz_sectors;
    uint32_t root_dir_first_cluster;
    char     fs_type[9];
    uint32_t signature;
    //
    uint32_t fat_begin_lba;
    uint32_t cluster_begin_lba;
    uint32_t cluster_szb;
} fat32_t;

int const mount(fat32_t *const fat32, uint8_t *const fat32_img);

typedef struct {
    uint32_t dir_first_cluster;
    uint32_t entry_number;

    char short_name[13];
    uint8_t  attribute;
    uint32_t first_cluster;
    uint32_t file_szb;
} fat32_entry_t;

void dir_set_root(fat32_t const *const fat32, fat32_entry_t *const fat32_entry);
void dir_set(fat32_entry_t *const fat32_entry, uint32_t const dir_first_cluster);
void dir_reset(fat32_entry_t *const fat32_entry);
int const get_entry(fat32_t const *const fat32, fat32_entry_t *const fat32_entry);
int const dir_descend(fat32_t const *const fat32, fat32_entry_t *const fat32_entry, char const dir_name[]);

int const dir_walk(fat32_t const *const fat32, fat32_entry_t *const fat32_entry, char const path[]);

typedef struct {
    struct fat32 *file_system;
    uint32_t first_cluster;
    uint8_t  attribute;
    int long file_szb;

    int long cursor;
    uint32_t cursor_cluster;
    uint32_t cursor_offset;
} fat32_file_t;

#define FAT32_SEEK_SET      0
#define FAT32_SEEK_CUR      1
#define FAT32_SEEK_END      2

fat32_file_t *const fat32_open(fat32_t *const fat32, char const path[]);
int const fat32_close(fat32_file_t *const fat32_file_handle);
int const fat32_seek(fat32_file_t *stream, int long const offset, int const origin_id);
int long const fat32_tell(fat32_file_t *const stream);
size_t const fat32_read(void *const buffer, size_t const size, size_t const count, fat32_file_t *stream);

#endif
