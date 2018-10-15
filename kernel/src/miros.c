#include <miros.h>

#include <syscall.h>

//--------------------------------------------------------------
// timer functions
//--------------------------------------------------------------
uint64_t const get_up_time_us(void) {
    uint32_t form[3];

    form[0] = SYSCALL_TIME_GET_UP_TIME_US;

    __asm__ volatile ("mv a0, %0; ecall" :: "r"(&form) : "a0", "memory");

    return *((uint64_t const *const)&(form[1]));
}
//--------------------------------------------------------------
// memory allocation functions
//--------------------------------------------------------------
uint32_t const mavailable(void) {
    uint32_t form[2];

    form[0] = SYSCALL_VMEM32_AVAILABLE;

    __asm__ volatile ("mv a0, %0; ecall" :: "r"(&form) : "a0", "memory");

    return form[1];
}
void *const malloc(size_t const szb) {
    uint32_t form[2];

    form[0] = SYSCALL_VMEM32_MALLOC;
    form[1] = (uint32_t const)szb;

    __asm__ volatile ("mv a0, %0; ecall" :: "r"(&form) : "a0", "memory");

    return (void *const)(form[1]);
}
void free(void *const buffer) {
    uint32_t form[2];

    form[0] = SYSCALL_VMEM32_FREE;
    form[1] = (uint32_t const)buffer;

    __asm__ volatile ("mv a0, %0; ecall" :: "r"(&form) : "a0", "memory");
}
//--------------------------------------------------------------
// thread functions
//--------------------------------------------------------------
unsigned const vthread_get_all(uint32_t **const list) {
    uint32_t form[2];

    form[0] = SYSCALL_VTHREAD_GETALL;
    form[1] = (uint32_t const)list;

    __asm__ volatile ("mv a0, %0; ecall" :: "r"(&form) : "a0", "memory");

    return (unsigned const)(form[1]);
}
uint32_t const vthread_create(void *const(*thread)(void *const), void *const arg) {
    uint32_t form[3];

    form[0] = SYSCALL_VTHREAD_CREATE;
    form[1] = (uint32_t const)thread;
    form[2] = (uint32_t const)arg;

    __asm__ volatile ("mv a0, %0; ecall" :: "r"(&form) : "a0", "memory");
}
//--------------------------------------------------------------
// file system functions
//--------------------------------------------------------------
fat32_t *const fs_mount(sd_context_t *const sd_context) {
    uint32_t form[2];

    form[0] = SYSCALL_FAT32_MOUNT;
    form[1] = (uint32_t const)sd_context;

    __asm__ volatile ("mv a0, %0; ecall" :: "r"(&form) : "a0", "memory");

    return (fat32_t *const)(form[1]);
}
int const fs_dir_get(fat32_entry_t *const dir_entry, char const path[]) {
    uint32_t form[3];

    form[0] = SYSCALL_FAT32_DIR_SET;
    form[1] = (uint32_t const)dir_entry;
    form[2] = (uint32_t const)path;

    __asm__ volatile ("mv a0, %0; ecall" :: "r"(&form) : "a0", "memory");

    return (int const)(form[1]);
}
int const fs_get_entry(fat32_entry_t *const fat32_entry) {
    uint32_t form[2];

    form[0] = SYSCALL_FAT32_GET_ENTRY;
    form[1] = (uint32_t const)fat32_entry;

    __asm__ volatile ("mv a0, %0; ecall" :: "r"(&form) : "a0", "memory");

    return (int const)(form[1]);
}
fat32_file_t *const fopen(char const path[]) {
    uint32_t form[2];

    form[0] = SYSCALL_FAT32_OPEN;
    form[1] = (uint32_t const)path;

    __asm__ volatile ("mv a0, %0; ecall" :: "r"(&form) : "a0", "memory");

    return (fat32_file_t *const)(form[1]);
}
int const fclose(fat32_file_t *const fat32_file_handle) {
    uint32_t form[2];

    form[0] = SYSCALL_FAT32_CLOSE;
    form[1] = (uint32_t const)fat32_file_handle;

    __asm__ volatile ("mv a0, %0; ecall" :: "r"(&form) : "a0", "memory");

    return (int const)(form[1]);
}
int const fseek(fat32_file_t *stream, int long const offset, int const origin_id) {
    uint32_t form[4];

    form[0] = SYSCALL_FAT32_SEEK;
    form[1] = (uint32_t const)stream;
    form[2] = (uint32_t const)offset;
    form[3] = (uint32_t const)origin_id;

    __asm__ volatile ("mv a0, %0; ecall" :: "r"(&form) : "a0", "memory");

    return (int const)(form[1]);
}
int long const ftell(fat32_file_t *const stream) {
    uint32_t form[2];

    form[0] = SYSCALL_FAT32_TELL;
    form[1] = (uint32_t const)stream;

    __asm__ volatile ("mv a0, %0; ecall" :: "r"(&form) : "a0", "memory");

    return (int long const)(form[1]);
}
size_t const fread(void *const buffer, size_t const size, size_t const count, fat32_file_t *stream) {
    uint32_t form[5];

    form[0] = SYSCALL_FAT32_READ;
    form[1] = (uint32_t const)buffer;
    form[2] = (uint32_t const)size;
    form[3] = (uint32_t const)count;
    form[4] = (uint32_t const)stream;

    __asm__ volatile ("mv a0, %0; ecall" :: "r"(&form) : "a0", "memory");

    return (size_t const)(form[1]);
}
