#include <fat32.h>

#include <vmem32.h>
#include <vmutex32.h>
#include <vstring.h>

#include <uart.h> // TODO - remove

#define DEV_BLOCK_BUFFER_SZBX       9
#define DEV_BLOCK_BUFFER_SZB        (1 << DEV_BLOCK_BUFFER_SZBX)
#define DEV_BLOCK_BUFFER_SZ_MASK    (DEV_BLOCK_BUFFER_SZB - 1)

#define FAT32_CLUSTER_ERROR         -1
#define FAT32_CLUSTER_EOF           1
#define FAT32_CLUSTER_USED          0

typedef struct {
    sd_context_t *sd_context; // change to general file handle later
    uint32_t dev_block_number;
    uint8_t *dev_block_buffer;
} dev_buffer_t;

struct fat32 {
    dev_buffer_t dev_buffer;
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
};

struct fat32_file {
    struct fat32 *file_system;
    uint32_t first_cluster;
    uint8_t  attribute;
    int long file_szb;

    int long cursor;
    uint32_t cursor_cluster;
    uint32_t cursor_offset;
};

static uint32_t const fat32_read_data(dev_buffer_t *const dev_buffer,
                                      size_t const size, uint32_t const offset) {
    uint32_t rd_data = 0;
    uint32_t block_number = offset >> DEV_BLOCK_BUFFER_SZBX;
    uint32_t block_offset = offset  & DEV_BLOCK_BUFFER_SZ_MASK;

    unsigned i;
    for (i = 0; i < (unsigned const)size; ++i, ++block_offset) {
        if (block_offset == DEV_BLOCK_BUFFER_SZB) {
            ++block_number;
            block_offset = 0;
        }
        // TODO - add error checking (device not available etc.)
        if (dev_buffer->dev_block_number != block_number) {
            sd_seek(dev_buffer->sd_context, (uint64_t const)offset & ~DEV_BLOCK_BUFFER_SZ_MASK, SD_SEEK_SET);
            sd_read(dev_buffer->dev_block_buffer, sizeof(uint8_t),
                    DEV_BLOCK_BUFFER_SZB, dev_buffer->sd_context);
            dev_buffer->dev_block_number = block_number;
        }

        rd_data |= ((uint32_t const)dev_buffer->dev_block_buffer[block_offset]) << (i << 3);
    }

    return rd_data;
}
static void fat32_dir_set_root(fat32_t const *const fat32, fat32_entry_t *const fat32_entry) {
    fat32_entry->dir_first_cluster = fat32->root_dir_first_cluster;
    fat32_entry->entry_number      = 0;
}
static void fat32_dir_set(fat32_entry_t *const fat32_entry, uint32_t const dir_first_cluster) {
    fat32_entry->dir_first_cluster = dir_first_cluster;
    fat32_entry->entry_number      = 0;
}
static void fat32_dir_reset(fat32_entry_t *const fat32_entry) {
    fat32_entry->entry_number = 0;
}
static int const fat32_get_next_cluster(fat32_t *const fat32,
                                  uint32_t *const next_cluster, uint32_t const current_cluster) {
    uint32_t const cluster = fat32_read_data(&fat32->dev_buffer, sizeof(uint32_t),
                                             fat32->fat_begin_lba * fat32->sector_szb
                                             + (current_cluster << 2));
    *next_cluster = cluster & 0x0fffffff;
    if (cluster == 0) {
        return FAT32_CLUSTER_ERROR;
    } else if (cluster >= 0xfffffff8) {
        return FAT32_CLUSTER_EOF;
    }
    return FAT32_CLUSTER_USED;
}
static int const fat32_get_coords(fat32_file_t *stream) {
    int long cursor;
    uint32_t cluster;
    int error;

    for (cursor = stream->cursor, cluster = stream->first_cluster, error = FAT32_CLUSTER_USED;
         error == FAT32_CLUSTER_USED && cursor > 0;
         cursor -= (int long const)stream->file_system->cluster_szb) {

        error = fat32_get_next_cluster(stream->file_system, &cluster, cluster);
    }
    if (cursor < 0) {
         cursor += (int long const)stream->file_system->cluster_szb;
    }
    stream->cursor_cluster = cluster;
    stream->cursor_offset  = cursor;

    return error;
}
static int const fat32_get_entry_i(fat32_t *const fat32, fat32_entry_t *const fat32_entry,
                             uint32_t const entry_number) {
    uint32_t entry_offset;
    uint32_t cluster;
    int error;

    for (entry_offset = entry_number << 5, cluster = fat32_entry->dir_first_cluster,
         error = FAT32_CLUSTER_USED;
         error == FAT32_CLUSTER_USED && entry_offset >= fat32->cluster_szb;
         entry_offset -= fat32->cluster_szb) {

        error = fat32_get_next_cluster(fat32, &cluster, cluster);
    }

    if (error == FAT32_CLUSTER_ERROR) {
        return 1;
    }

    uint32_t const record_offset = (fat32->cluster_begin_lba + (cluster - 2) * fat32->cluster_sz_sectors)
                                 * fat32->sector_szb + entry_offset;

    int in_ext = 0;
    uint32_t i;
    unsigned j;

    for (i = 0, j = 0; i < 11; ++i) {
        char const c = (char const)fat32_read_data(&fat32->dev_buffer, sizeof(uint8_t), record_offset + i);
        if (c != ' ') {
            if (i > 7 && !in_ext) {
                in_ext = 1;
                fat32_entry->short_name[j++] = '.';
            }
            fat32_entry->short_name[j++] = c;
        }
    }
    fat32_entry->short_name[j] = '\0';
    fat32_entry->attribute     = fat32_read_data(&fat32->dev_buffer, sizeof(uint8_t), record_offset + 11);
    fat32_entry->first_cluster = fat32_read_data(&fat32->dev_buffer, sizeof(uint16_t), record_offset + 26);
    fat32_entry->first_cluster = fat32_entry->first_cluster
                               | (fat32_read_data(&fat32->dev_buffer, sizeof(uint16_t),
                                                  record_offset + 20) << 16);
    //printf("\nRD:%X", record_offset);
    fat32_entry->file_szb      = fat32_read_data(&fat32->dev_buffer, sizeof(uint32_t), record_offset + 28);

    return fat32_entry->short_name[0] == 0;
}
static char const *const fat32_dir_descend(fat32_t *const fat32, fat32_entry_t *const fat32_entry,
                                     char const dir_name[]) {
    int match = 0;
    int i;

    fat32_dir_reset(fat32_entry);

    while (match == 0 && !fat32_get_entry(fat32, fat32_entry)) {
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
        fat32_dir_set(fat32_entry, fat32->root_dir_first_cluster);
    } else {
        fat32_dir_set(fat32_entry, fat32_entry->first_cluster);
    }

    return (match ? &dir_name[i] : NULL);
}
//--------------------------------------------------------------
// externally visible functions
//--------------------------------------------------------------
fat32_t *const fat32_mount(sd_context_t *const sd_context) {
    unsigned i;
    int error = 0;

    fat32_t *const fat32 = (fat32_t *const)vmem32_alloc(sizeof(fat32_t));
    if (fat32 == NULL) {
        return NULL;
    }
    fat32->dev_buffer.sd_context = sd_context;
    fat32->dev_buffer.dev_block_number = 0;
    fat32->dev_buffer.dev_block_buffer = (uint8_t *const)vmem32_alloc(DEV_BLOCK_BUFFER_SZB);
    if (fat32->dev_buffer.dev_block_buffer == NULL) {
        vmem32_free(fat32);
        return NULL;
    }
    error |= sd_seek(sd_context, 0, SD_SEEK_SET);
    uint32_t const read_count = sd_read(fat32->dev_buffer.dev_block_buffer, sizeof(uint8_t),
                                        DEV_BLOCK_BUFFER_SZB, sd_context);
    if (read_count != DEV_BLOCK_BUFFER_SZB) {
        error |= 1;
    }

    fat32->sector_szb             = fat32_read_data(&fat32->dev_buffer, sizeof(uint16_t), 11);
    fat32->cluster_sz_sectors     = fat32_read_data(&fat32->dev_buffer, sizeof(uint8_t), 13);
    fat32->rsvd_sector_count      = fat32_read_data(&fat32->dev_buffer, sizeof(uint16_t), 14); // FAT32 : 32
    fat32->fat_count              = fat32_read_data(&fat32->dev_buffer, sizeof(uint8_t), 16);
    fat32->fat_sz_sectors         = fat32_read_data(&fat32->dev_buffer, sizeof(uint32_t), 36);
    fat32->root_dir_first_cluster = fat32_read_data(&fat32->dev_buffer, sizeof(uint32_t), 44);
    for (i = 0; i < 8; ++i) {
        fat32->fs_type[i] = fat32_read_data(&fat32->dev_buffer, sizeof(uint8_t), 82 + i);
    }
    fat32->fs_type[8] = '\0';
    fat32->signature  = fat32_read_data(&fat32->dev_buffer, sizeof(uint16_t), 510);

    fat32->fat_begin_lba     = fat32->rsvd_sector_count;
    fat32->cluster_begin_lba = fat32->rsvd_sector_count + (fat32->fat_count * fat32->fat_sz_sectors);
    fat32->cluster_szb       = fat32->cluster_sz_sectors * fat32->sector_szb;

    // check it's a supported file system
    for (i = 0; i < 8; ++i) {
        error = error | ((fat32->fs_type[i] != "FAT32   "[i]) ? 1 : 0);
    }
    error = error | ((fat32->signature != 0xaa55) ? 2 : 0);

    if (error) {
        vmem32_free(fat32->dev_buffer.dev_block_buffer);
        vmem32_free(fat32);
        return NULL;
    }

    return fat32;
}
int const fat32_dir_get(fat32_t *const fat32, fat32_entry_t *const dir_entry, char const path[]) {
    // TODO - remove requirment for trailing '/'
    if (fat32 == NULL) {
        return 1;
    }

    int i;
    int match;
    char const *sub_path;

    fat32_dir_set_root(fat32, dir_entry);

    for (sub_path = path, match = 1; match && sub_path[0] != '\0'; ) {
        for (i = 0; sub_path[i] == '/'; ++i);
        sub_path = &sub_path[i];
        //printf("<subpath=%s>", sub_path);
        if (sub_path[0] != '\0') {
            sub_path = fat32_dir_descend(fat32, dir_entry, sub_path);
            match = (sub_path == NULL ? 0 : 1);
            //printf("<match=%u>", match);
        }
    }

    if (!match) {
        return 2;
    }

    return 0;
}
int const fat32_get_entry(fat32_t *const fat32, fat32_entry_t *const fat32_entry) {
    uint32_t entry_number = fat32_entry->entry_number;

    do {
        fat32_get_entry_i(fat32, fat32_entry, entry_number++);
    } while ((fat32_entry->short_name[0] != 0x00 && (fat32_entry->attribute & 0xf) == 0xf)
             || fat32_entry->short_name[0] == 0xe5);

    if (fat32_entry->short_name[0] == 0) {
        fat32_entry->entry_number = 0;
        return 1;
    } else {
        fat32_entry->entry_number = entry_number;
        return 0;
    }
}
fat32_file_t *const fat32_open(fat32_t *const fat32, char const path[]) {
    if (fat32 == NULL) {
        return NULL;
    }
    int i;
    int match;
    int found_dir;
    char const *file_name;
    char const *sub_path;
    fat32_entry_t dir_entry;

    fat32_dir_set_root(fat32, &dir_entry);

    for (i = strlen(path); i >= 0 && path[i] != '/'; --i);
    file_name = (i < 0 ? path : &path[i + 1]);

    for (sub_path = path, match = 1, found_dir = 0; !found_dir && match; ) {
        for (i = 0; sub_path[i] == '/' && &sub_path[i] != file_name; ++i);
        sub_path = &sub_path[i];
        if (sub_path == file_name) {
            found_dir = 1;
        } else {
            sub_path = fat32_dir_descend(fat32, &dir_entry, sub_path);
            match = (sub_path == NULL ? 0 : 1);
        }
    }
    if (!found_dir) {
        return NULL;
    }
    //printf("\nDF<%s>", file_name);

    int eod;
    match = 0;
    do {
        eod = fat32_get_entry(fat32, &dir_entry);
        //printf("\nEOD(%i)", eod);
        if (!(dir_entry.attribute & FAT32_ENTRY_ATTRIB_DIR)
            && !strcmp(dir_entry.short_name, file_name)) {
            //printf("\nEOD:M", eod);
            match = 1;
        }
    } while (match == 0 && !eod);

    if (!match) {
        return NULL;
    }
    //printf("\nM");

    fat32_file_t *const fat32_file_handle = (fat32_file_t *const)vmem32_alloc(sizeof(fat32_file_t));
    fat32_file_handle->file_system   = fat32;
    fat32_file_handle->first_cluster = dir_entry.first_cluster;
    fat32_file_handle->attribute     = dir_entry.attribute;
    fat32_file_handle->file_szb      = (int long const)dir_entry.file_szb;
    fat32_file_handle->cursor        = 0;
    return fat32_file_handle;
}
int const fat32_close(fat32_file_t *const fat32_file_handle) {
    if (fat32_file_handle == NULL) {
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
        //printf("S:%u\n", stream->file_szb);
        stream->cursor = stream->file_szb + offset;
    } else {
        stream->cursor = offset;
    }
    return 0;
}
int long const fat32_tell(fat32_file_t *const stream) {
    //printf("T:%u\n", stream->cursor);
    return stream->cursor;
}
size_t const fat32_read(void *const buffer, size_t const size, size_t const count, fat32_file_t *stream) {
    //vmutex32_wait_for_lock();

    if (fat32_get_coords(stream) == FAT32_CLUSTER_ERROR) {
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

        if (cursor_offset == stream->file_system->cluster_szb) {
            cursor_offset = 0;
            state = fat32_get_next_cluster(stream->file_system, &cursor_cluster, cursor_cluster);
        }
        ((uint8_t *const)buffer)[i] = fat32_read_data(&stream->file_system->dev_buffer, sizeof(uint8_t),
                                            ((stream->file_system->cluster_sz_sectors
                                              * (cursor_cluster - 2) + stream->file_system->cluster_begin_lba)
                                             * stream->file_system->sector_szb)
                                            + cursor_offset);
    }
    stream->cursor = cursor;

    //vmutex32_unlock();

    return (size_t const)i;
}
