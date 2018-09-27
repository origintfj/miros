#ifndef MIROS_H
#define MIROS_H

#include <stddef.h>

// memory allocation functions
void *const malloc(size_t const szb);
void free(void *const buffer);
// thread functions
/*
thread_handle_t const vthread32_create(void *const(*thread)(void *const), void *const arg,
                                       unsigned const stack_szw, uint32_t const mstatus);
*/
// string functions
int const strcmp(char const str1[], char const str2[]);

#endif
