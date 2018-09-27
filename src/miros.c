#include <miros.h>

#include <syscall.h>
#include <stdint.h>

//--------------------------------------------------------------
// memory allocation functions
//--------------------------------------------------------------
void *const malloc(size_t const szb) {
    uint32_t request_form[2];

    request_form[0] = SYSCALL_MALLOC;
    request_form[1] = (uint32_t const)szb;

    __asm__ volatile ("mv a0, %0; ecall" :: "r"(&request_form) : "a0", "memory");

    return (void *const)(request_form[1]);
}
void free(void *const buffer) {
    uint32_t request_form[2];

    request_form[0] = SYSCALL_FREE;
    request_form[1] = (uint32_t const)buffer;

    __asm__ volatile ("mv a0, %0; ecall" :: "r"(&request_form) : "a0", "memory");
}
//--------------------------------------------------------------
// string functions
//--------------------------------------------------------------
int const strcmp(char const str1[], char const str2[]) {
    int i;
    for (i = 0; str1[i] == str2[i] && str1[i] != '\0'; ++i);
    return str1[i] == str2[i];
}
