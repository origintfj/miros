#include <fat32.h>

#include <miros.h>

#include <uart.h> // TODO - remove

uint32_t const read8(uint8_t const *const buffer, uint32_t const offset) {
    return (uint32_t const)buffer[offset];
}
uint32_t const read16(uint8_t const *const buffer, uint32_t const offset) {
    return (((uint32_t const)buffer[offset + 1] << 8) |
            ((uint32_t const)buffer[offset + 0] << 0) ) & 0xffff;
}
uint32_t const read32(uint8_t const *const buffer, uint32_t const offset) {
    return ((uint32_t const)buffer[offset + 3] << 24) | ((uint32_t const)buffer[offset + 2] << 16) |
           ((uint32_t const)buffer[offset + 1] <<  8) | ((uint32_t const)buffer[offset + 0] <<  0) ;
}

int const mount(fs_info_t *const fs_info, uint8_t *const fat32_img) {
    unsigned i;
    int error = 0;

    fs_info->fs_img = fat32_img;

    fs_info->sector_szb             = read16(fat32_img, 11);
    fs_info->cluster_sz_sectors     = read8( fat32_img, 13);
    fs_info->rsvd_sector_count      = read16(fat32_img, 14); // FAT32 : 32
    fs_info->fat_count              = read8( fat32_img, 16);
    fs_info->fat_sz_sectors         = read32(fat32_img, 36);
    fs_info->root_dir_first_cluster = read32(fat32_img, 44);
    for (i = 0; i < 8; ++i) {
        fs_info->fs_type[i] = read8(fat32_img, 82 + i);
    }
    fs_info->fs_type[8] = '\0';
    fs_info->signature  = read16(fat32_img, 510);

    fs_info->fat_begin_lba     = fs_info->rsvd_sector_count;
    fs_info->cluster_begin_lba = fs_info->rsvd_sector_count + (fs_info->fat_count * fs_info->fat_sz_sectors);

    // check it's a supported file system
    for (i = 0; i < 8; ++i) {
        error = error | ((fs_info->fs_type[i] != "FAT32   "[i]) ? 1 : 0);
    }
    error = error | ((fs_info->signature != 0xaa55) ? 2 : 0);

    return error;
}

int const get_entry_number(fs_info_t const *const fs_info, dir_record_t *const dir_record,
                           uint32_t const entry_number) {
    uint32_t record_offset = (fs_info->cluster_begin_lba + (dir_record->dir_first_cluster - 2)
                           * fs_info->cluster_sz_sectors)
                           * fs_info->sector_szb + (entry_number << 5);

    int in_ext = 0;
    uint32_t i;
    unsigned j;

    for (i = 0, j = 0; i < 11; ++i) {
        char const c = (char const)read8(fs_info->fs_img, record_offset + i);
        if (c != ' ') {
            if (i > 7 && !in_ext) {
                in_ext = 1;
                dir_record->short_name[j++] = '.';
            }
            dir_record->short_name[j++] = c;
        }
    }
    dir_record->short_name[j] = '\0';
    dir_record->attribute = read8(fs_info->fs_img, record_offset + 11);
    dir_record->first_cluster = read16(fs_info->fs_img, record_offset + 26);
    dir_record->first_cluster = dir_record->first_cluster
                              | (read16(fs_info->fs_img, record_offset + 20) << 16);
    dir_record->file_szb = read32(fs_info->fs_img, record_offset + 28);

    return dir_record->short_name[0] == 0;
}

void dir_set_root(fs_info_t const *const fs_info, dir_record_t *const dir_record) {
    dir_record->dir_first_cluster = fs_info->root_dir_first_cluster;
    dir_record->entry_number      = 0;
}
void dir_set(dir_record_t *const dir_record, uint32_t const dir_first_cluster) {
    dir_record->dir_first_cluster = dir_first_cluster;
    dir_record->entry_number      = 0;
}
void dir_reset(dir_record_t *const dir_record) {
    dir_record->entry_number = 0;
}
int const get_entry(fs_info_t const *const fs_info, dir_record_t *const dir_record) {
    uint32_t entry_number = dir_record->entry_number;

    do {
        get_entry_number(fs_info, dir_record, entry_number++);
    } while (dir_record->short_name[0] != 0 && (dir_record->attribute & 0xf) == 0xf);

    if (dir_record->short_name[0] == 0) {
        dir_record->entry_number = 0;
        return 1;
    } else {
        dir_record->entry_number = entry_number;
        return 0;
    }
}
int const dir_descend(fs_info_t const *const fs_info, dir_record_t *const dir_record,
                      char const dir_name[]) {
    int match = 0;

    dir_reset(dir_record);

    while (match == 0 && !get_entry(fs_info, dir_record)) {
        //printf("Found '%s'\n", dir_record->short_name);
        if ((dir_record->attribute & FAT32_DIR_ATTRIB_DIR) && !strcmp(dir_record->short_name, dir_name)) {
            match = 1;
        }
    }
    if (dir_record->first_cluster == 0) { // TODO - is this really required to fix FAT32
        dir_set(dir_record, fs_info->root_dir_first_cluster);
    } else {
        dir_set(dir_record, dir_record->first_cluster);
    }

    return match == 0;
}
int const dir_walk(fs_info_t const *const fs_info, dir_record_t *const dir_record, char const path[]) {
    unsigned i = 0;
    unsigned finished = 0;
    int error = 0;
    unsigned start;

    char *const temp_path = malloc(strlen(path) + 1);

    strcpy(temp_path, path);

    dir_set_root(fs_info, dir_record);

    while (!finished && !error) {
        start = i;
        for (; temp_path[i] != '/' && temp_path[i] != '\0'; ++i);
        if (temp_path[i] == '\0') {
            finished = 1;
        } else {
            temp_path[i] = '\0';
        }
        ++i;
        if (temp_path[start] != '\0') {
            //printf("->'%s'\n", &temp_path[start]);
            error = dir_descend(fs_info, dir_record, &temp_path[start]);
        }
    }

    free(temp_path);

    return error;
}
