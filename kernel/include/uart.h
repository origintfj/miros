#ifndef UART_H
#define UART_H

void uart_init(unsigned const baudrate);
void uart_flush_rx(void);
void uart_putc(char const c);
char const uart_getc(void);
void uart_puts(char const* str);
void uart_putx(int const value);

void printf(char const* const fmt, ...);

#endif
