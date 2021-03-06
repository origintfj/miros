#include <vstring.h>

int const strcmp(char const str1[], char const str2[]) {
    int i;
    for (i = 0; str1[i] == str2[i] && str1[i] != '\0'; ++i);
    return str1[i] != str2[i];
}
size_t const strlen(char const str[]) {
    unsigned i;
    for (i = 0; str[i] != '\0'; ++i);
    return (size_t const)i;
}
char *const strcpy(char str_dest[], char const str_src[]) {
    int i;
    for (i = 0; str_src[i] != '\0'; ++i) {
        str_dest[i] = str_src[i];
    }
    str_dest[i] = '\0';
    return str_dest;
}
char *const strcat(char str_dest[], char const str_src[]) {
    unsigned i, j;
    for (i = 0; str_dest[i] != '\0'; ++i);
    for (j = 0; str_src[j] != '\0'; ++j, ++i) {
        str_dest[i] = str_src[j];
    }
    str_dest[i] = '\0';
    return str_dest;
}
char *const strlwr(char str_dest[], char const str_src[]) {
    int i;
    for (i = 0; str_src[i] != '\0'; ++i) {
        str_dest[i] = (str_src[i] >= 'A' && str_src[i] <= 'Z' ? str_src[i] - 'A' + 'a' : str_src[i]);
    }
    str_dest[i] = '\0';
    return str_dest;
}
char *const strupr(char str_dest[], char const str_src[]) {
    int i;
    for (i = 0; str_src[i] != '\0'; ++i) {
        str_dest[i] = (str_src[i] >= 'a' && str_src[i] <= 'z' ? str_src[i] - 'a' + 'A' : str_src[i]);
    }
    str_dest[i] = '\0';
    return str_dest;
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
