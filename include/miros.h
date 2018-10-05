#ifndef MIROS_H
#define MIROS_H

#include <stddef.h>
#include <stdint.h>

// timer functions
uint64_t const get_up_time_us(void);
// memory allocation functions
uint32_t const mavailable(void);
void *const malloc(size_t const szb);
void free(void *const buffer);
// thread functions
unsigned const vthread_get_all(uint32_t **const list);
uint32_t const vthread_create(void *const(*thread)(void *const), void *const arg);
// string functions
int const strcmp(char const str1[], char const str2[]);
size_t const strlen(char const str[]);
char *const strcpy(char str_dest[], char const str_src[]);
int const atoi(char const str[]);
int const xtoi(char const str[]);

#endif
