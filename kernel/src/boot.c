#include <stdint.h>
#include <soc.h>
#include <common.h>
#include <uart.h>
#include <vmem32.h>
#include <vthread32.h>

#define STACK_SZW   1000u

// TODO - move this
#include <fat32.h>
fat32_t *fat32_root_fs; // TODO - move to time module

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

#include <sd.h>

void *const boot_thread(void *const arg) {
    printf("In boot_thread.\n");
    vthread32_create(gpio_time, NULL, 1024u, 0x1880);
    vthread32_create(shell, NULL, 1024u, 0x1880);

    //vmem32_dump_table();

    return NULL;
}
/*
int const execute(int const argc, char const *const *const argv,
                  char const *const buffer) {
    char *str_buffer;
    void *arg_buffer;
    char const **arg_vector;
    int i;
    int error;

    arg_buffer = vmem32_alloc((strlen(buffer) + 1) * (1 + sizeof(char *)));
    //printf("\nbuffer = %X\n", (uint32_t const)arg_buffer);

    // set the str_buffer pointer to the start of the arg_buffer
    str_buffer = (char *const)arg_buffer;
    // set the arg_vector pointer to that section of the arg_buffer
    arg_vector = (char const **const)((char *const)arg_buffer + BUFFER_SZB);
    // copy the string to the str_buffer
    for (i = 0; buffer[i] != '\0'; ++i) {
        str_buffer[i] = buffer[i];
    }
    str_buffer[i] = '\0';
    // copy the argv to the argv
    for (i = 0; i < argc; ++i) {
        arg_vector[i] = argv[i] + (arg_vector - argv);
    }

    //printf("\nExecuting from the arg list (count = %i)\n", argc);
    for (i = 0; i < argc; ++i) {
        //printf("'%s'\n", argv[i]);
    }
    error = run(argc, argv);
    if (error) {
        start(argc, argv);
    }

    vmem32_free(arg_buffer);
    return error;
}
*/
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
    systime_us += 100;
    vthread32_switch();
    write_reg(TIMER0_BASE_ADDR + TIMER0_INT_OFFSET, 0);
}
