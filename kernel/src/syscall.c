#include <syscall.h>

#include <vmem32.h>
#include <vthread32.h>
#include <sd.h>
#include <fat32.h>
#include <vprocess32.h>

extern fat32_t *fat32_root_fs; // TODO - move to a module

//#include <uart.h>

void syscall(uint32_t *const argv) {
    if (argv[0] == SYSCALL_TIME_GET_UP_TIME_US) {
        //uint64_t const uptime_us = time_us(); TODO
        //argv[1] = (uint32_t const)(uptime_us >>  0);
        //argv[2] = (uint32_t const)(uptime_us >> 32);
    } else if (argv[0] == SYSCALL_VMEM32_AVAILABLE) {
        argv[1] = (uint32_t const)vmem32_available();
    } else if (argv[0] == SYSCALL_VMEM32_MALLOC) {
        argv[1] = (uint32_t const)vmem32_alloc((size_t const)(argv[1]));
    } else if (argv[0] == SYSCALL_VMEM32_FREE) {
        vmem32_free((void *const)(argv[1]));
    } else if (argv[0] == SYSCALL_VTHREAD_FINISHED) {
        vthread32_finished_handler(argv[1]);
    } else if (argv[0] == SYSCALL_VTHREAD_GETALL) {
        argv[1] = (uint32_t const)vthread32_get_all((thread_handle_t **const)(argv[1]));
    } else if (argv[0] == SYSCALL_VTHREAD_CREATE) {
        thread_id_t thread_id;
        thread_id = vthread32_create((void *const(*)(void *const))argv[1],
                                     (void *const)argv[2], 1024u, 0x0080);
        argv[2] = (uint32_t const)(thread_id >> 32);
        argv[1] = (uint32_t const)(thread_id >>  0);
    } else if (argv[0] == SYSCALL_VTHREAD_JOIN) {
        argv[1] = (uint32_t const)vthread32_join((uint64_t const)argv[2] << 32 | (uint64_t const)argv[1] << 0,
                                                 (void **const)argv[3]);
    } else if (argv[0] == SYSCALL_FAT32_MOUNT) {
        //argv[1] = (uint32_t const)fat32_mount((void *const)argv[1]); // TODO
        fat32_root_fs = fat32_mount((sd_context_t *const)argv[1]);
    } else if (argv[0] == SYSCALL_FAT32_UMOUNT) { // TODO - implement
    } else if (argv[0] == SYSCALL_FAT32_DIR_SET) {
        argv[1] = (uint32_t const)fat32_dir_get(fat32_root_fs, (fat32_entry_t *const)argv[1],
                                                (char const *const)argv[2]);
    } else if (argv[0] == SYSCALL_FAT32_GET_ENTRY) {
        argv[1] = (uint32_t const)fat32_get_entry(fat32_root_fs, (fat32_entry_t *const)argv[1]);
    } else if (argv[0] == SYSCALL_FAT32_OPEN) {
        argv[1] = (uint32_t const)fat32_open(fat32_root_fs, (char const *const)argv[1]);
    } else if (argv[0] == SYSCALL_FAT32_CLOSE) {
        argv[1] = (uint32_t const)fat32_close((fat32_file_t *const)argv[1]);
    } else if (argv[0] == SYSCALL_FAT32_SEEK) {
        argv[1] = (uint32_t const)fat32_seek((fat32_file_t *)argv[1], (int long const)argv[2],
                                             (int const)argv[3]);
    } else if (argv[0] == SYSCALL_FAT32_TELL) {
        argv[1] = (uint32_t const)fat32_tell((fat32_file_t *const)argv[1]);
    } else if (argv[0] == SYSCALL_FAT32_READ) {
        argv[1] = (uint32_t const)fat32_read((void *const)argv[1], (size_t const)argv[2],
                                             (size_t const)argv[3], (fat32_file_t *)argv[4]);
    } else if (argv[0] == SYSCALL_PROC_CREATE) {
        thread_id_t thread_id;
        thread_id = vprocess32_start(fat32_root_fs, (char const *const)argv[1], 1024u, (int const)argv[2],
                                     (char const *const *const)argv[3]);
        argv[2] = (uint32_t const)(thread_id >> 32);
        argv[1] = (uint32_t const)(thread_id >>  0);
    }
}
