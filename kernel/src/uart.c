#include <soc.h> // TODO - remove

#include <uart.h>
#include <common.h>
#include <cqueue.h>

#define UART_DATA_OFFSET            0
#define UART_CPHB_OFFSET            4
#define UART_STATUS_OFFSET          8
#define UART_INT_EN_OFFSET          12
#define UART_INT_STATE_OFFSET       16
//
#define UART_STATUS_RXDAV_MASK      8
#define UART_STATUS_RXFULL_MASK     4
#define UART_STATUS_TXBUSY_MASK     2
#define UART_STATUS_TXFULL_MASK     1

#define WRITE_REG(A, D)     *((uint32_t volatile *const)(A)) = (D)
#define READ_REG(A)         *((uint32_t volatile const *const)(A))

cqueue_t rx_queue;
cqueue_t tx_queue;

void uart_init(unsigned const cphb) {
    WRITE_REG(UART_BASE_ADDR + UART_CPHB_OFFSET, (uint32_t const)cphb);
    cqueue_init(&rx_queue);
    cqueue_init(&tx_queue);
}
void uart_putc(char const c) {
    while (cqueue_full(&tx_queue));
    cqueue_putc(&tx_queue, c);
}
char const uart_getc(void) {
    while (cqueue_empty(&rx_queue));
    return cqueue_getc(&rx_queue);
}
void uart_flush_tx(void) {
    while (!cqueue_empty(&tx_queue) &&
           !(READ_REG(UART_BASE_ADDR + UART_STATUS_OFFSET) & UART_STATUS_TXFULL_MASK)) {
        WRITE_REG(UART_BASE_ADDR + UART_DATA_OFFSET, (uint32_t const)cqueue_getc(&tx_queue));
    }
}
void uart_flush_rx(void) {
    while (!cqueue_full(&rx_queue) &&
           (READ_REG(UART_BASE_ADDR + UART_STATUS_OFFSET) & UART_STATUS_RXDAV_MASK)) {
        cqueue_putc(&rx_queue, (char const)READ_REG(UART_BASE_ADDR + UART_DATA_OFFSET));
    }
}
