#include <soc.h>
#include <uart.h>
#include <basicio.h>

void printf(char const* const fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(uart_putc, fmt, args);
    va_end(args);
}

#include <common.h>

#define TIMER_TIME_OFFSET           0
#define TIMER_MAX_OFFSET            4
#define TIMER_CTRL_OFFSET           8
#define TIMER_INT_OFFSET            12
//
#define TIMER_CTRL_INT_EN_MASK      0x2
#define TIMER_CTRL_EN_MASK          0x1

void mtip(void) {
    WRITE_REG(TIMER0_BASE_ADDR + TIMER_INT_OFFSET, 0);
    uart_flush_tx();
    uart_flush_rx();
    //printf("TIP ");
}

uint32_t const syscall(uint32_t arg);

uint32_t const syscall(uint32_t const arg) {
    printf("%x\n", arg);
    return 0;
}

//#define __SYSCALL(A)    __asm__ volatile ("li a0, " A "; ecall" ::: "a0", "memory");

void timer_init(void) {
    WRITE_REG(TIMER0_BASE_ADDR + TIMER_MAX_OFFSET, 25000 - 1);
    WRITE_REG(TIMER0_BASE_ADDR + TIMER_TIME_OFFSET, 0);
    WRITE_REG(TIMER0_BASE_ADDR + TIMER_INT_OFFSET, 0);
    WRITE_REG(TIMER0_BASE_ADDR + TIMER_CTRL_OFFSET, TIMER_CTRL_EN_MASK | TIMER_CTRL_INT_EN_MASK);
}

void boot(void) {
    uart_init(CLK_FREQ / 115200 / 2);

    timer_init();
    __asm__ volatile ("li t0, 0x880; csrw mie, t0; li t0, 0x8; csrs mstatus, t0" ::: "t0", "memory");

    printf("MiROS %i\n", 1650);
    printf("MiROS %i\n", 1650);

    //__SYSCALL(0xabc);

    while(1) {
        uart_putc(uart_getc());
    }
}
