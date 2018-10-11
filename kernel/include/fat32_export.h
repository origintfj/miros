#ifndef FAT32_EXPORT_H
#define FAT32_EXPORT_H

#define FAT32_ENTRY_ATTRIB_DIR      0x10 // TODO move this
#define FAT32_MAX_PATH_LENGTH       256

#define FAT32_SEEK_SET              0
#define FAT32_SEEK_CUR              1
#define FAT32_SEEK_END              2

typedef struct fat32 fat32_t;

typedef struct fat32_entry {
    uint32_t dir_first_cluster;
    uint32_t entry_number;

    char short_name[13];
    uint8_t  attribute;
    uint32_t first_cluster;
    uint32_t file_szb;
} fat32_entry_t;

typedef struct fat32_file fat32_file_t;

#endif
