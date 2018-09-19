#include <soc.h>
#include <common.h>
#include <uart.h>
#include <vmem32.h>
#include <vthread32.h>

#define STACK_SZW   1000u

extern uint8_t _mpool_start[];

uint64_t volatile systime_us;

void *const boot_thread(void *const arg) {
    printf("In boot_thread.\n");
    return NULL;
}

extern uint32_t const _got_start[];
extern uint32_t const _got_end[];

void boot(void) {
    int error;
    uint32_t const *i;

    printf("GOT (%X-%X:\n", _got_start, _got_end);
    for (i = _got_start; i < _got_end; ++i) {
        printf("%X: %X\n", i, *i);
    }

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
    printf("mpool start = 0x%X\n", (uint32_t const)_mpool_start);
    vmem32_init(_mpool_start, (uint8_t *const)(SRAM_END_ADDR - 1));

    vmem32_dump_table();
    printf("\n");

    printf("Initialising multi-threading...");
    error = vthread32_init(boot_thread, NULL, STACK_SZW, STACK_SZW);
    printf("Done! (%i)\n", error);
    __asm__ volatile ("csrw mstatus, 0x8" ::: "memory");
    printf("Done! (%i)\n", error);

    vmem32_dump_table();
    printf("\n");

    //while (1) printf("%i\n", 23876423);

    while (1); printf(".");
}

void trap_usip_excp(void) {
    printf("Possible Exception!\n");
    while (1);
}
void trap_ssip(void) {
}
void trap_msip(void) {
}
void trap_utip(void) {
}
void trap_stip(void) {
}
void trap_mtip(void) {
    systime_us += 100;

    vthread32_switch();

    write_reg(TIMER0_BASE_ADDR + TIMER0_INT_OFFSET, 0);
}
void trap_ueip(void) {
}
void trap_seip(void) {
}
void trap_meip(void) {
}
