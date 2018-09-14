#include <uart.h>
#include <soc.h>
#include <common.h>

void uart_init(unsigned const baudrate) {
    write_reg(UART_BASE_ADDR + UART_CPHB_OFFSET, (CLK_FREQ / baudrate) >> 1);
}
void uart_putc(char const c) {
    while (read_reg(UART_BASE_ADDR + UART_STATUS_OFFSET) & UART_STATUS_TXFULL_MASK);
    write_reg(UART_BASE_ADDR + UART_DATA_OFFSET, (uint32_t const)c);
}
char const uart_getc(void) {
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

void uart_printf(char const* const fmt, ...) {
}
