#include <basicio.h>

static void print_x(void (*putc)(char const), unsigned const value) {
    if (value == 0) {
        return;
    }
    print_x(putc, value >> 4);
    putc("0123456789abcdef"[value & 0xf]);
}
static void print_X(void (*putc)(char const), unsigned const value) {
    if (value == 0) {
        return;
    }
    print_x(putc, value >> 4);
    putc("0123456789ABCDEF"[value & 0xf]);
}
static void print_u(void (*putc)(char const), unsigned const value) {
    if (value < 10) {
        putc("0123456789"[value]);
        return;
    }
    print_u(putc, value / 10);
    putc("0123456789"[value % 10]);
}
static void print_i(void (*putc)(char const), int const value) {
    if (value < 0) {
        putc('-');
        print_u(putc, (unsigned const)(~value + 1));
    } else {
        print_u(putc, (unsigned const)value);
    }
}
static void print_s(void (*putc)(char const), char const str[]) {
    int i;

    for (i = 0; str[i] != '\0'; ++i) {
        putc(str[i]);
    }
}
void vfprintf(void (*putc)(char const), char const* const fmt, va_list args) {
    int i;

    int  found;
    //char pad_value;
    //int  length;

    found = 0;

    for (i = 0; fmt[i] != '\0'; ++i) {
        if (found) {
            //pad_value = fmt[i];
            switch (fmt[i]) {
                case 'c': putc(va_arg(args, int const));
                          break;
                case 'x': print_x(putc, va_arg(args, unsigned const));
                          break;
                case 'X': print_X(putc, va_arg(args, unsigned const));
                          break;
                case 'u': print_u(putc, va_arg(args, unsigned const));
                          break;
                case 'i': print_i(putc, va_arg(args, int const));
                          break;
                case 's': print_s(putc, va_arg(args, char const *const));
                          break;
                default : putc(fmt[i]);
                          break;
            }
            found = 0;
        } else if (fmt[i] == '%') {
            found = 1;
        } else {
            if (fmt[i] == '\n') {
                putc('\r');
            }
            putc(fmt[i]);
        }
    }
}
