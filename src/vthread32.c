#include <vthread32.h>

#include <vmem32.h>

#define VTHREAD32_CONTEXT_SZW   30

struct thread_info {
    void *container;
    uint32_t mstatus;
    uint32_t mepc;
    uint32_t const *stack_base_ed;
    //
    struct thread_info *previous;
    struct thread_info *next;
};

typedef struct thread_info thread_info_t;
typedef thread_info_t *thread_handle_t;

thread_info_t dummy;

thread_handle_t thread_ring;
thread_handle_t active_thread;

#include <uart.h>

void vthread32_exit_handler(void) {
    printf("In exit_handler.\n");
    while (1);
}

int const vthread32_init(void *const(*thread)(void *const),
                         void *const arg, unsigned const stack_szw,
                         unsigned const kernel_stack_szw) {
    uint32_t *kernel_stack_base_ed;
    uint32_t *thread_stack_base_ed;
    void *thread_container;

    kernel_stack_base_ed = vmem32_alloc(kernel_stack_szw << 2)
                         + kernel_stack_szw;
    if (kernel_stack_base_ed == VMEM32_NULL) {
        return 1;
    }
    printf("Kernel stack = 0x%x\n", kernel_stack_base_ed);

    thread_ring = vmem32_alloc(sizeof(thread_info_t));
    if (thread_ring == VMEM32_NULL) {
        vmem32_free(kernel_stack_base_ed);
        return 2;
    }
    printf("thread_ring = 0x%x\n", thread_ring);

    thread_stack_base_ed = (uint32_t *const)(thread_container = vmem32_alloc(stack_szw << 2))
                         + stack_szw - VTHREAD32_CONTEXT_SZW;
    if (thread_stack_base_ed == VMEM32_NULL) {
        vmem32_free(kernel_stack_base_ed);
        vmem32_free(thread_ring);
        return 3;
    }

    __asm__ volatile ("csrw mscratch, %0" :: "r"(kernel_stack_base_ed) : "memory");

    thread_stack_base_ed[0] = (uint32_t const)vthread32_exit_handler;
    thread_stack_base_ed[8] = (uint32_t const)arg;

    // initialise the thread info. struct
    thread_ring->container     = thread_container;
    thread_ring->mstatus       = 0x1880;
    thread_ring->mepc          = (uint32_t const)thread;
    thread_ring->stack_base_ed = thread_stack_base_ed;
    //
    thread_ring->previous = thread_ring;
    thread_ring->next     = thread_ring;

    dummy.next = thread_ring;
    active_thread = &dummy;
    printf("In switch, active = 0x%X, ", active_thread);

    return 0;
}
void vthread32_switch(void) {
    uint32_t mstatus;
    uint32_t mepc;
    uint32_t const *stack_base_ed;

    __asm__ volatile ("csrr %0, mstatus" : "=r"(mstatus) :: "memory");
    __asm__ volatile ("csrr %0, mepc" : "=r"(mepc) :: "memory");
    __asm__ volatile ("csrr %0, mscratch" : "=r"(stack_base_ed) :: "memory");
    active_thread->mstatus       = mstatus;
    active_thread->mepc          = mepc;
    active_thread->stack_base_ed = stack_base_ed;

    //uart_putc('<');//printf("In switch, active = 0x, ");
    //printf("In switch, active = 0x%X, ", active_thread);
    //putc();
    active_thread = active_thread->next;
    //putc();
    //uart_putc('>');//printf("next = 0x\n");
    //printf("next = 0x%X\n", active_thread);

    mstatus       = active_thread->mstatus;
    mepc          = active_thread->mepc;
    stack_base_ed = active_thread->stack_base_ed;
    __asm__ volatile ("csrw mstatus, %0" :: "r"(mstatus) : "memory");
    __asm__ volatile ("csrw mepc, %0" :: "r"(mepc) : "memory");
    __asm__ volatile ("csrw mscratch, %0" :: "r"(stack_base_ed) : "memory");
}
/*
    __asm__ volatile ("csrr %0, mstatus" : "=r"(mstatus) :: "memory");
    __asm__ volatile ("csrr %0, mepc" : "=r"(mepc) :: "memory");
    __asm__ volatile ("csrr %0, mscratch" : "=r"(stack_base_ed) :: "memory");
    printf("Switch start: mstatus=0x%x, mepc=0x%x, stack=0x%x\n", mstatus, mepc, stack_base_ed);
*/
