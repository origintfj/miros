#ifndef FAT32_H
#define FAT32_H

#include <stdint.h>

#define FAT32_DIR_ATTRIB_DIR    0x10

typedef struct {
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
} fs_info_t;

int const mount(fs_info_t *const fs_info, uint8_t *const fat32_img);

typedef struct {
    uint32_t dir_first_cluster;
    uint32_t entry_number;

    char short_name[13];
    uint8_t attribute;
    uint32_t first_cluster;
    uint32_t file_szb;
} dir_record_t;

void dir_set_root(fs_info_t const *const fs_info, dir_record_t *const dir_record);
void dir_set(dir_record_t *const dir_record, uint32_t const dir_first_cluster);
void dir_reset(dir_record_t *const dir_record);
int const get_entry(fs_info_t const *const fs_info, dir_record_t *const dir_record);
int const dir_descend(fs_info_t const *const fs_info, dir_record_t *const dir_record, char const dir_name[]);

int const dir_walk(fs_info_t const *const fs_info, dir_record_t *const dir_record, char const path[]);

#endif
