#include <uart.h>
#include <soc.h>
#include <common.h>

void uart_init(unsigned const baudrate) {
    write_reg(UART_BASE_ADDR + UART_CPHB_OFFSET, (CLK_FREQ / baudrate) >> 1);
}
void uart_flush_rx(void) {
    while (read_reg(UART_BASE_ADDR + UART_STATUS_OFFSET) & UART_STATUS_RXDAV_MASK) {
        read_reg(UART_BASE_ADDR + UART_DATA_OFFSET);
    }
}
void uart_putc(char const c) {
    if (c == '\n') {
        while (read_reg(UART_BASE_ADDR + UART_STATUS_OFFSET) & UART_STATUS_TXFULL_MASK);
        write_reg(UART_BASE_ADDR + UART_DATA_OFFSET, '\r');
    }
    while (read_reg(UART_BASE_ADDR + UART_STATUS_OFFSET) & UART_STATUS_TXFULL_MASK);
    write_reg(UART_BASE_ADDR + UART_DATA_OFFSET, (uint32_t const)c);
}
char const uart_getc(void) {
    while ((read_reg(UART_BASE_ADDR + UART_STATUS_OFFSET) & UART_STATUS_RXDAV_MASK) == 0);
    return (char const)read_reg(UART_BASE_ADDR + UART_DATA_OFFSET);
}
void uart_puts(char const* str) {
    int i;

    for (i = 0; str[i] != '\0'; ++i) {
        while (read_reg(UART_BASE_ADDR + UART_STATUS_OFFSET) & UART_STATUS_TXFULL_MASK);
        write_reg(UART_BASE_ADDR + UART_DATA_OFFSET, (uint32_t const)str[i]);
    }
}
void uart_putx(int const value) {
}

#include <stdarg.h>

static void print_x(void (*putc)(char const), uint32_t const value) {
    int i;

    for (i = 7; i >= 0; --i) {
        putc("0123456789abcdef"[(value >> (i << 2)) & 0xf]);
    }
}
static void print_X(void (*putc)(char const), uint32_t const value) {
    int i;

    for (i = 7; i >= 0; --i) {
        putc("0123456789ABCDEF"[(value >> (i << 2)) & 0xf]);
    }
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

static void vfprintf(void (*putc)(char const), char const* const fmt, va_list args) {
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
                case 'x': print_x(putc, va_arg(args, uint32_t const));
                          break;
                case 'X': print_X(putc, va_arg(args, uint32_t const));
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
            putc(fmt[i]);
        }
    }
}
void printf(char const* const fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(uart_putc, fmt, args);
    va_end(args);
}
