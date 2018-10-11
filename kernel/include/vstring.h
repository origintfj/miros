#ifndef VSTRING_H
#define VSTRING_H

#include <stddef.h>

int const strcmp(char const str1[], char const str2[]);
size_t const strlen(char const str[]);
char *const strcpy(char str_dest[], char const str_src[]);
char *const strcat(char str_dest[], char const str_src[]);
char *const strlwr(char str_dest[], char const str_src[]);
char *const strupr(char str_dest[], char const str_src[]);
int const atoi(char const str[]);
int const xtoi(char const str[]);

#endif
