#ifndef UART_H
#define UART_H

void uart_init(unsigned const cphb);
void uart_putc(char const c);
char const uart_getc(void);
void uart_flush_tx(void);
void uart_flush_rx(void);

#endif
