#include <stdint.h>
#include <soc.h>
#include <common.h>
#include <uart.h>
#include <vmem32.h>
#include <vthread32.h>

#define STACK_SZW   1000u

extern uint8_t _mpool_start[];

uint64_t volatile systime_us;

uint64_t const time_us(void) {
    return systime_us;
}

void *const boot_thread(void *const arg);

void boot(void) {
    int error;
    uint32_t const *i;

    printf("MiROS\n\n");

    // setup the timer interrupt with a 100us interval
    systime_us = 0;
    write_reg(TIMER0_BASE_ADDR + TIMER0_MAX_OFFSET, CLK_FREQ / 10000 - 1); // 100us
    write_reg(TIMER0_BASE_ADDR + TIMER0_TIME_OFFSET, 0);
    write_reg(TIMER0_BASE_ADDR + TIMER0_INT_OFFSET, 0);
    write_reg(TIMER0_BASE_ADDR + TIMER0_CTRL_OFFSET, TIMER0_CTRL_EN_MASK | TIMER0_CTRL_INT_EN_MASK);
    // enable the m-timer interrupt
    __asm__ volatile ("li t0, 0x80; csrs mie, t0" ::: "t0", "memory");


    // initialise dynamic memory allocation
    printf("Initalising dynamic memory allocation...");
    vmem32_init(_mpool_start, (uint8_t *const)(SRAM_END_ADDR - 1));
    printf("Done!\n");

    printf("Initialising multi-threading...");
    error = vthread32_init(boot_thread, NULL, STACK_SZW, STACK_SZW);
    printf("Done! (%i)\n", error);
    VTHREAD32_HANDOFF

    __builtin_unreachable();
}

void *const gpio_time(void *const arg) {
    uint64_t systime;

    while (1) {
        systime = time_us();
        write_reg(GPIO_BASE_ADDR + GPIO_IO_OFFSET, systime);
    }
    return NULL;
}
void *const thread1(void *const arg) {
    int i;

    for (i = 0; i < 100000; ++i) {
        write_reg(GPIO_BASE_ADDR + GPIO_IO_OFFSET, i); // 100us
        //printf("in thread 1.\n");
    }
}
void *const thread2(void *const arg) {
    int i;

    for (i = 0; i < 100000; ++i) {
        write_reg(GPIO_BASE_ADDR + GPIO_IO_OFFSET, i); // 100us
        //printf("in thread 2.\n");
    }
}
void *const thread3(void *const arg) {
    int i;

    for (i = 0; i < 100000; ++i) {
        write_reg(GPIO_BASE_ADDR + GPIO_IO_OFFSET, i); // 100us
        //printf("in thread 3.\n");
    }
}

void *const boot_thread(void *const arg) {
    printf("In boot_thread.\n");
    vthread32_create(gpio_time, NULL, 1024u, 0x1880);
    vthread32_create(thread1, NULL, 1024u, 0x1880);
    vthread32_create(thread2, NULL, 1024u, 0x1880);
    vthread32_create(thread3, NULL, 1024u, 0x1880);

    vmem32_dump_table();
    printf("\n");

    while (time_us() < 1000000);

    vmem32_dump_table();
    printf("\n");

    return NULL;
}

void mtrap_handler(void) {
    uint32_t mcause;

    __asm__ volatile ("csrr %0, mcause" : "=r"(mcause) :: "memory");

    systime_us += 100;

    vthread32_switch();

    write_reg(TIMER0_BASE_ADDR + TIMER0_INT_OFFSET, 0);
}
