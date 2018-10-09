#include <fat32.h>

#include <miros.h> // TODO - for string functions only, remove and replace
#include <vmem32.h>
#include <vmutex32.h>

#include <uart.h> // TODO - remove

uint32_t const read8(uint8_t const *const buffer, uint32_t const offset) {
    return (uint32_t const)buffer[offset];
}
uint32_t const read16(uint8_t const *const buffer, uint32_t const offset) {
    return ((uint32_t const)buffer[offset + 1] << 8) |
           ((uint32_t const)buffer[offset + 0] << 0) ;
}
uint32_t const read32(uint8_t const *const buffer, uint32_t const offset) {
    return ((uint32_t const)buffer[offset + 3] << 24) | ((uint32_t const)buffer[offset + 2] << 16) |
           ((uint32_t const)buffer[offset + 1] <<  8) | ((uint32_t const)buffer[offset + 0] <<  0) ;
}

int const mount(fat32_t *const fat32, uint8_t *const fat32_img) {
    unsigned i;
    int error = 0;

    fat32->fs_img = fat32_img;

    fat32->sector_szb             = read16(fat32_img, 11);
    fat32->cluster_sz_sectors     = read8( fat32_img, 13);
    fat32->rsvd_sector_count      = read16(fat32_img, 14); // FAT32 : 32
    fat32->fat_count              = read8( fat32_img, 16);
    fat32->fat_sz_sectors         = read32(fat32_img, 36);
    fat32->root_dir_first_cluster = read32(fat32_img, 44);
    for (i = 0; i < 8; ++i) {
        fat32->fs_type[i] = read8(fat32_img, 82 + i);
    }
    fat32->fs_type[8] = '\0';
    fat32->signature  = read16(fat32_img, 510);

    fat32->fat_begin_lba     = fat32->rsvd_sector_count;
    fat32->cluster_begin_lba = fat32->rsvd_sector_count + (fat32->fat_count * fat32->fat_sz_sectors);
    fat32->cluster_szb       = fat32->cluster_sz_sectors * fat32->sector_szb;

    // check it's a supported file system
    for (i = 0; i < 8; ++i) {
        error = error | ((fat32->fs_type[i] != "FAT32   "[i]) ? 1 : 0);
    }
    error = error | ((fat32->signature != 0xaa55) ? 2 : 0);

    return error;
}

int const get_entry_number(fat32_t const *const fat32, fat32_entry_t *const fat32_entry,
                           uint32_t const entry_number) {
    uint32_t record_offset = (fat32->cluster_begin_lba + (fat32_entry->dir_first_cluster - 2)
                           * fat32->cluster_sz_sectors)
                           * fat32->sector_szb + (entry_number << 5);

    int in_ext = 0;
    uint32_t i;
    unsigned j;

    for (i = 0, j = 0; i < 11; ++i) {
        char const c = (char const)read8(fat32->fs_img, record_offset + i);
        if (c != ' ') {
            if (i > 7 && !in_ext) {
                in_ext = 1;
                fat32_entry->short_name[j++] = '.';
            }
            fat32_entry->short_name[j++] = c;
        }
    }
    fat32_entry->short_name[j] = '\0';
    fat32_entry->attribute = read8(fat32->fs_img, record_offset + 11);
    fat32_entry->first_cluster = read16(fat32->fs_img, record_offset + 26);
    fat32_entry->first_cluster = fat32_entry->first_cluster
                               | (read16(fat32->fs_img, record_offset + 20) << 16);
    printf("\nRD:%X", record_offset);
    fat32_entry->file_szb = read32(fat32->fs_img, record_offset + 28);

    return fat32_entry->short_name[0] == 0;
}

void dir_set_root(fat32_t const *const fat32, fat32_entry_t *const fat32_entry) {
    fat32_entry->dir_first_cluster = fat32->root_dir_first_cluster;
    fat32_entry->entry_number      = 0;
}
void dir_set(fat32_entry_t *const fat32_entry, uint32_t const dir_first_cluster) {
    fat32_entry->dir_first_cluster = dir_first_cluster;
    fat32_entry->entry_number      = 0;
}
void dir_reset(fat32_entry_t *const fat32_entry) {
    fat32_entry->entry_number = 0;
}
int const get_entry(fat32_t const *const fat32, fat32_entry_t *const fat32_entry) {
    uint32_t entry_number = fat32_entry->entry_number;

    do {
        get_entry_number(fat32, fat32_entry, entry_number++);
    } while (fat32_entry->short_name[0] != 0 && (fat32_entry->attribute & 0xf) == 0xf);

    if (fat32_entry->short_name[0] == 0) {
        fat32_entry->entry_number = 0;
        return 1;
    } else {
        fat32_entry->entry_number = entry_number;
        return 0;
    }
}
int const dir_descend(fat32_t const *const fat32, fat32_entry_t *const fat32_entry,
                      char const dir_name[]) {
    int match = 0;

    dir_reset(fat32_entry);

    while (match == 0 && !get_entry(fat32, fat32_entry)) {
        //printf("Found '%s'\n", fat32_entry->short_name);
        if ((fat32_entry->attribute & FAT32_ENTRY_ATTRIB_DIR) && !strcmp(fat32_entry->short_name, dir_name)) {
            match = 1;
        }
    }
    if (fat32_entry->first_cluster == 0) { // TODO - is this really required to fix FAT32
        dir_set(fat32_entry, fat32->root_dir_first_cluster);
    } else {
        dir_set(fat32_entry, fat32_entry->first_cluster);
    }

    return match == 0;
}
int const dir_walk(fat32_t const *const fat32, fat32_entry_t *const fat32_entry, char const path[]) {
    unsigned i = 0;
    unsigned finished = 0;
    int error = 0;
    unsigned start;

    char *const temp_path = vmem32_alloc(strlen(path) + 1);

    strcpy(temp_path, path);

    dir_set_root(fat32, fat32_entry);

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
            error = dir_descend(fat32, fat32_entry, &temp_path[start]);
        }
    }

    free(temp_path);

    return error;
}
char const *const fat32_dir_descend(fat32_t const *const fat32, fat32_entry_t *const fat32_entry,
                                    char const dir_name[]) {
    int match = 0;
    int i;

    dir_reset(fat32_entry);

    while (match == 0 && !get_entry(fat32, fat32_entry)) {
        //printf("Found '%s'\n", fat32_entry->short_name);
        if ((fat32_entry->attribute & FAT32_ENTRY_ATTRIB_DIR)) {
            for (i = 0; fat32_entry->short_name[i] == dir_name[i]; ++i);
            if (fat32_entry->short_name[i] == '\0'
                && (dir_name[i] =='\0' || dir_name[i] =='/')) {
                for (; dir_name[i] == '/'; ++i);
                match = 1;
            } else {
                match = 0;
            }
        }
    }
    if (fat32_entry->first_cluster == 0) { // TODO - is this really required to fix FAT32
        dir_set(fat32_entry, fat32->root_dir_first_cluster);
    } else {
        dir_set(fat32_entry, fat32_entry->first_cluster);
    }

    return (match ? &dir_name[i] : NULL);
}
fat32_file_t *const fat32_open(fat32_t *const fat32, char const path[]) {
    if (fat32 == VMEM32_NULL) {
        return VMEM32_NULL;
    }
    int i;
    int match;
    int found_dir;
    char const *file_name;
    char const *sub_path;
    fat32_entry_t dir_entry;

    dir_set_root(fat32, &dir_entry);

    for (i = strlen(path); i > 0 && path[i] != '/'; --i);
    file_name = (i == 0 ? path : &path[i + 1]);

    for (sub_path = path, match = 1, found_dir = 0; !found_dir && match; ) {
        for (i = 0; sub_path[i] == '/' && &sub_path[i] != file_name; ++i);
        sub_path = &sub_path[i];
        //printf("<subpath=%s>", sub_path);
        if (sub_path == file_name) {
            found_dir = 1;
        } else {
            sub_path = fat32_dir_descend(fat32, &dir_entry, sub_path);
            match = (sub_path == NULL ? 0 : 1);
        }
    }
    if (!found_dir) {
        return VMEM32_NULL;
    }
    while (match == 0 && !get_entry(fat32, &dir_entry)) {// TODO rename this function
        //printf("Found '%s'\n", fat32_entry->short_name);
        if (!(dir_entry.attribute & FAT32_ENTRY_ATTRIB_DIR)
            && !strcmp(dir_entry.short_name, file_name)) {
            match = 1;
        }
    }
    if (!match) {
        return VMEM32_NULL;
    }
    fat32_file_t *const fat32_file_handle = (fat32_file_t *const)vmem32_alloc(sizeof(fat32_file_t));
    fat32_file_handle->file_system   = fat32;
    fat32_file_handle->first_cluster = dir_entry.first_cluster;
    fat32_file_handle->attribute     = dir_entry.attribute;
    fat32_file_handle->file_szb      = (int long const)dir_entry.file_szb;
    fat32_file_handle->cursor        = 0;
    printf("O:%u\n", dir_entry.file_szb);
    return fat32_file_handle;
}
int const fat32_close(fat32_file_t *const fat32_file_handle) {
    if (fat32_file_handle == VMEM32_NULL) {
        return 1;
    }
    // TODO flush outstanding writes etc...
    vmem32_free(fat32_file_handle);
    return 0;
}
int const fat32_seek(fat32_file_t *stream, int long const offset, int const origin_id) {
    if (origin_id == FAT32_SEEK_CUR) {
        stream->cursor = stream->cursor + offset;
    } else if (origin_id == FAT32_SEEK_END) {
        printf("S:%u\n", stream->file_szb);
        stream->cursor = stream->file_szb + offset;
    } else {
        stream->cursor = offset;
    }
    return 0;
}
int long const fat32_tell(fat32_file_t *const stream) {
    printf("T:%u\n", stream->cursor);
    return stream->cursor;
}
#define FAT32_CLUSTER_ERROR     -1
#define FAT32_CLUSTER_EOF       1
#define FAT32_CLUSTER_USED      0
int const fat32_get_next_cluster(fat32_file_t const *const stream,
                                 uint32_t *const next_cluster, uint32_t const current_cluster) {
    uint32_t const cluster = read32(stream->file_system->fs_img,
                                    stream->file_system->cluster_begin_lba * stream->file_system->sector_szb
                                    + (current_cluster << 2));
    *next_cluster = cluster & 0x0fffffff;
    if (cluster == 0) {
        return -1;
    } else if (cluster >= 0xfffffff8) {
        return 1;
    }
    return 0;
}
int const fat32_get_coords(fat32_file_t *stream) {
    int long cursor;
    uint32_t cluster;
    int error;

    printf("COORDS: cluster=%X\n", stream->first_cluster);
    for (cursor = stream->cursor, cluster = stream->first_cluster, error = 0;
         error == 0 && cursor > 0;
         cursor -= (int long const)stream->file_system->cluster_szb) {

        error = fat32_get_next_cluster(stream, &cluster, cluster);
    }
    if (cursor < 0) {
         cursor += (int long const)stream->file_system->cluster_szb;
    }
    stream->cursor_cluster = cluster;
    stream->cursor_offset  = cursor;

    return error;
}
size_t const fat32_read(void *const buffer, size_t const size, size_t const count, fat32_file_t *stream) {
    //vmutex32_wait_for_lock();

    if (fat32_get_coords(stream)) {
        return (size_t const)0;
    }

    int long cursor         = stream->cursor;
    uint32_t cursor_cluster = stream->cursor_cluster;
    uint32_t cursor_offset  = stream->cursor_offset;

    unsigned i;
    int state;

    for (i = 0, state = FAT32_CLUSTER_USED;
         state == FAT32_CLUSTER_USED && i < count * size;
         ++i, ++cursor, ++cursor_offset) {
    printf("cluster=%X, offset=%u\n", cursor_cluster, cursor_offset);

        if (cursor_offset == stream->file_system->cluster_szb) {
            cursor_offset = 0;
            state = fat32_get_next_cluster(stream, &cursor_cluster, cursor_cluster);
        }
        ((uint8_t *const)buffer)[i] = read8(stream->file_system->fs_img,
                                            ((stream->file_system->cluster_sz_sectors
                                              * cursor_cluster + stream->file_system->cluster_begin_lba)
                                             * stream->file_system->sector_szb)
                                            + cursor_offset);
    }
    stream->cursor = cursor;

    //vmutex32_unlock();

    return (size_t const)i;
}
