#include <stdio.h>

#include "fat32fs.h"

#include <fat32.h>


int const fat32_ls(fs_info_t const *const fat32_fs, dir_record_t *const dir_record) {
    while (!get_entry(fat32_fs, dir_record)) {
        if (dir_record->attribute & FAT32_DIR_ATTRIB_DIR) {
            printf("               DIR");
        } else {
            printf("%10u byte(s)", dir_record->file_szb);
        }
        printf("    %c%s%c\n", dir_record->attribute & FAT32_DIR_ATTRIB_DIR ? '[' : '\'',
                                   dir_record->short_name,
                                   dir_record->attribute & FAT32_DIR_ATTRIB_DIR ? ']' : '\'');//,
                                   //dir_record->first_cluster);
    }
    return 0;
}

int main(void) {
    int i;
    uint8_t *fat32_img;
    fs_info_t fat32_fs;

    fat32_img = (uint8_t *const)fat32fs_img;

    printf("Mounting...(%u)\n", mount(&fat32_fs, fat32_img));

    dir_record_t dir_record;

    dir_set_root(&fat32_fs, &dir_record);
    fat32_ls(&fat32_fs, &dir_record);

    char const *path;

    path = "/";
    printf("\ndescending to '%s'\n", path);
    dir_walk(&fat32_fs, &dir_record, path);
    fat32_ls(&fat32_fs, &dir_record);

    path = "//ROOT//SUB//";
    printf("\ndescending to '%s'\n", path);
    dir_walk(&fat32_fs, &dir_record, path);
    fat32_ls(&fat32_fs, &dir_record);

    path = "";
    printf("\ndescending to '%s'\n", path);
    dir_walk(&fat32_fs, &dir_record, path);
    fat32_ls(&fat32_fs, &dir_record);

    path = "SUB/ROOT";
    printf("\ndescending to '%s'\n", path);
    dir_walk(&fat32_fs, &dir_record, path);
    fat32_ls(&fat32_fs, &dir_record);

    path = "ROOT/SUB/";
    printf("\ndescending to '%s'\n", path);
    dir_walk(&fat32_fs, &dir_record, path);
    fat32_ls(&fat32_fs, &dir_record);

    // read the BPB

    printf("fat32_fs.sector_szb             (%u)\n", fat32_fs.sector_szb);
    printf("fat32_fs.cluster_sz_sectors     (%u)\n", fat32_fs.cluster_sz_sectors);
    printf("fat32_fs.rsvd_sector_count      (%u)\n", fat32_fs.rsvd_sector_count);
    printf("fat32_fs.fat_count              (%u)\n", fat32_fs.fat_count);
    printf("fat32_fs.fat_sz_sectors         (%u)\n", fat32_fs.fat_sz_sectors);
    printf("fat32_fs.root_dir_first_cluster (%u)\n", fat32_fs.root_dir_first_cluster);
    printf("fat32_fs.fs_type '%s'", fat32_fs.fs_type);
    printf("fat32_fs.signature (0x%X)\n", fat32_fs.signature);
    printf("fat32_fs.cluster_begin_lba      (%u)\n", fat32_fs.cluster_begin_lba);

    return 0;
}
