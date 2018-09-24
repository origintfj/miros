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
    write_reg(TIMER0_BASE_ADDR + TIMER0_CTRL_OFFSET,
              TIMER0_CTRL_EN_MASK | TIMER0_CTRL_INT_EN_MASK);
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

void *const shell(void *const arg);

void *const boot_thread(void *const arg) {
    printf("In boot_thread.\n");
    vthread32_create(gpio_time, NULL, 1024u, 0x1880);
    vthread32_create(shell, NULL, 1024u, 0x1880);

    return NULL;
}

void trap_usip_excp(void) {
    printf("Possible exception.\n");
    while(1);
    __builtin_unreachable();
}
void mtrap_handler(void *const argv, int32_t const mcause) {
    if (mcause < 0) { // interrupr
        if ((mcause & 0xf) == 7) { // mtip
            systime_us += 100;
            vthread32_switch();
            write_reg(TIMER0_BASE_ADDR + TIMER0_INT_OFFSET, 0);
        }
    } else { // exception
        __asm__ volatile ("csrr t0, mepc; addi t0, t0, 4; csrw mepc, t0" ::: "t0", "memory");
        vthread32_append_job(vthread32_get_active(), argv);
        if (mcause == 11) { // m-ecall
        } else if (mcause == 9) { // s-ecall
        } else if (mcause == 8) { // u-ecall
        }
    }
}
