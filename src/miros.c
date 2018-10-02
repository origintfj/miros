#include <miros.h>

#include <syscall.h>

//--------------------------------------------------------------
// timer functions
//--------------------------------------------------------------
uint64_t const get_up_time_us(void) {
    uint32_t form[2];

    form[0] = SYSCALL_TIME_GET_UP_TIME_US;

    __asm__ volatile ("mv a0, %0; ecall" :: "r"(&form) : "a0", "memory");

    return ((uint64_t const)(form[2]) << 32) | (uint64_t const)(form[1]);
}
//--------------------------------------------------------------
// memory allocation functions
//--------------------------------------------------------------
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
// string functions
//--------------------------------------------------------------
int const strcmp(char const str1[], char const str2[]) {
    int i;
    for (i = 0; str1[i] == str2[i] && str1[i] != '\0'; ++i);
    return str1[i] == str2[i];
}
int const atoi(char const str[]) {
    int num = 0;
    int neg = 0;
    int i;

    i = 0;
    if (str[i] == '\0') {
        return 0;
    } else if (str[i] == '-') {
        neg = 1;
        ++i;
    } else if (str[i] == '+') {
        ++i;
    }
    for (; str[i] != '\0'; ++i) {
        num *= 10;
        num += (int const)(str[i] - '0');
    }
    return (neg ? -num : num);
}
int const xtoi(char const str[]) {
    int i;
    int digit;
    int out = 0;

    for (i = 0; str[i] != '\0'; ++i) {
        if (str[i] >= '0' && str[i] <= '9') {
            digit = (int const)(str[i] - '0');
        } else if (str[i] >= 'a' && str[i] <= 'f') {
            digit = (int const)(str[i] - 'a' + 10);
        } else if (str[i] >= 'A' && str[i] <= 'F') {
            digit = (int const)(str[i] - 'A' + 10);
        }

        out = out << 4;
        out = out | (digit & 0xff);
    }

    return out;
}
