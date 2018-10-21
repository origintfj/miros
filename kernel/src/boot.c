#include <stdint.h>
#include <soc.h>
#include <common.h>
#include <vtime32.h>
#include <uart.h>
#include <vmem32.h>
#include <vthread32.h>

#define STACK_SZW   1000u

// TODO - move this
#include <fat32.h>
fat32_t *fat32_root_fs; // TODO - move to time module

vtime32_context_t timer0;

extern uint8_t _mpool_start[];

void *const boot_thread(void *const arg);

void boot(void) {
    int error;
    uint32_t const *i;

    printf("MiROS\n\n");

    // setup the timer interrupt with a 100us interval
    vtime32_init(&timer0, TIMER0_BASE_ADDR, CLK_FREQ, 100);

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
    unsigned hex_time;

    while (1) {
        hex_time = (timer0.time.days / 10) << 28
                 | (timer0.time.days % 10) << 24
                 | (timer0.time.hours / 10) << 20
                 | (timer0.time.hours % 10) << 16
                 | (timer0.time.minutes / 10) << 12
                 | (timer0.time.minutes % 10) <<  8
                 | (timer0.time.seconds / 10) <<  4
                 | (timer0.time.seconds % 10) <<  0
                 ;
        write_reg(GPIO_BASE_ADDR + GPIO_IO_OFFSET, (uint32_t const)hex_time);
    }
    return NULL;
}

void *const shell(void *const arg);

#include <sd.h>

void *const boot_thread(void *const arg) {
    printf("%c[0m", 0x1b);
    printf("In boot_thread.\n");
    vthread32_create(gpio_time, NULL, 1024u, 0x1880);

    printf("\nCreating SD Context for device @ %X...", SPIM_BASE_ADDR);
    sd_context_t *const sd_context = sd_context_create(SPIM_BASE_ADDR, CLK_FREQ);
    if (sd_context == NULL) {
        printf("FAILED!");
        return NULL;
    }
    printf("SUCCESSS (%X)", (uint32_t const)sd_context);

    printf("\nAttempting FAT32 file system mount with SD context %X...", (uint32_t const)sd_context);
    fat32_root_fs = fat32_mount(sd_context);
    if (fat32_root_fs == NULL) {
        printf("FAILED!");
        return NULL;
    }
    printf("SUCCESSS (%X)", (uint32_t const)fat32_root_fs);

    vthread32_create(shell, NULL, 1024u, 0x0080);

    //vmem32_dump_table();

    return NULL;
}

#include <syscall.h>

void trap_excp(uint32_t *const argv, int32_t const mcause) {
    uint32_t mepc;
    uint32_t mstatus;

    if (mcause == 11) { // m-ecall
        __asm__ volatile ("csrr %0, mepc; csrrs %1, mstatus, 0x8" : "=r"(mepc), "=r"(mstatus) :: "memory");
        mepc += 4;
        syscall(argv);
        __asm__ volatile ("csrw mstatus, %0; csrw mepc, %1" :: "r"(mstatus), "r"(mepc) : "memory");
    } else if (mcause == 9) { // s-ecall
        __asm__ volatile ("csrr %0, mepc; csrrs %1, mstatus, 0x8" : "=r"(mepc), "=r"(mstatus) :: "memory");
        mepc += 4;
        syscall(argv);
        __asm__ volatile ("csrw mstatus, %0; csrw mepc, %1" :: "r"(mstatus), "r"(mepc) : "memory");
    } else if (mcause == 8) { // u-ecall
        __asm__ volatile ("csrr %0, mepc; csrrs %1, mstatus, 0x8" : "=r"(mepc), "=r"(mstatus) :: "memory");
        mepc += 4;
        syscall(argv);
        __asm__ volatile ("csrw mstatus, %0; csrw mepc, %1" :: "r"(mstatus), "r"(mepc) : "memory");
    } else {
        unsigned i;
        int count;
        uint32_t addr;

        __asm__ volatile ("csrr %0, mepc; mv %1, sp" : "=r"(mepc), "=r"(addr) :: "memory");

        printf("\n[%X] Exception (%u).\n", mepc, mcause);
        count = 256;
        printf("Dumping %i word(s) from address 0x%X:\n", count, addr);
        for (i = 0; i < (unsigned const)count; ++i) {
            if ((i & 3) == 0) {
                printf("\n%X: ", (addr + i * sizeof(uint32_t)));
            }
            printf("%X ", ((uint32_t const *const)addr)[i]);
        }
        while(1);
        __builtin_unreachable();
    }
}
void trap_usip(void) {
    printf("USIP.\n");
    while(1);
    __builtin_unreachable();
}
void trap_mtip(void) {
    vtime32_isr(&timer0);
    vthread32_switch();
}
