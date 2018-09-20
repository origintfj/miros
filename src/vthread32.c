#include <vthread32.h>

#include <stdint.h>
#include <vmem32.h>
#include <vmutex32.h>

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

thread_info_t dummy;

thread_handle_t thread_ring;
thread_handle_t active_thread;

vmutex32_t      new_thread_mutex;
thread_handle_t volatile new_thread;

vmutex32_t      finished_thread_mutex;
thread_handle_t volatile finished_thread;

vmutex32_t      dead_thread_mutex;
thread_handle_t volatile dead_thread;

#include <uart.h>

void vthread32_finished_handler(void) {
    //printf("In finished_handler.\n");
    vmutex32_wait_for_lock(&finished_thread_mutex, VMUTEX32_STATE_LOCKED);
    while (finished_thread != VMEM32_NULL);
    finished_thread = active_thread;
    vmutex32_unlock(&finished_thread_mutex);

    while (1);
    __builtin_unreachable();
}
void *const vthread32_cleanup_deamon(void *const arg) {
    //printf("Starting cleanup deamon-----------.\n");
    while (1) {
        while (dead_thread == VMEM32_NULL);
        //printf("Running cleanup on thread 0x%X.\n", (uint32_t const)dead_thread);
        vmem32_free(dead_thread->container);
        vmem32_free(dead_thread);
        dead_thread = VMEM32_NULL;
    }

    __builtin_unreachable();
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
    //printf("Kernel stack = 0x%x\n", kernel_stack_base_ed);

    thread_ring = vmem32_alloc(sizeof(thread_info_t));
    if (thread_ring == VMEM32_NULL) {
        vmem32_free(kernel_stack_base_ed);
        return 2;
    }
    //printf("thread_ring = 0x%x\n", thread_ring);

    thread_stack_base_ed = (uint32_t *const)(thread_container = vmem32_alloc(stack_szw << 2))
                         + stack_szw - VTHREAD32_CONTEXT_SZW;
    if (thread_stack_base_ed == VMEM32_NULL) {
        vmem32_free(kernel_stack_base_ed);
        vmem32_free(thread_ring);
        return 3;
    }

    __asm__ volatile ("csrw mscratch, %0" :: "r"(kernel_stack_base_ed) : "memory");

    thread_stack_base_ed[0] = (uint32_t const)vthread32_finished_handler;
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
    //printf("In switch, active = 0x%X, ", active_thread);

    vmutex32_init(&new_thread_mutex);
    new_thread = VMEM32_NULL;
    vmutex32_init(&finished_thread_mutex);
    finished_thread = VMEM32_NULL;
    vmutex32_init(&dead_thread_mutex);
    dead_thread = VMEM32_NULL;

    vthread32_create(vthread32_cleanup_deamon, NULL, 1024u, 0x1880);

    return 0;
}
thread_handle_t const vthread32_create(void *const(*thread)(void *const), void *const arg,
                                       unsigned const stack_szw, uint32_t const mstatus) {
    uint32_t *thread_stack_base_ed;
    void *thread_container;
    thread_handle_t temp_thread;

    temp_thread = vmem32_alloc(sizeof(thread_info_t));
    if (temp_thread == VMEM32_NULL) {
        return VMEM32_NULL;
    }
    //printf("temp_thread = 0x%x\n", temp_thread);

    thread_stack_base_ed = (uint32_t *const)(thread_container = vmem32_alloc(stack_szw << 2))
                         + stack_szw - VTHREAD32_CONTEXT_SZW;
    if (thread_stack_base_ed == VMEM32_NULL) {
        vmem32_free(temp_thread);
        return VMEM32_NULL;
    }

    thread_stack_base_ed[0] = (uint32_t const)vthread32_finished_handler;
    thread_stack_base_ed[8] = (uint32_t const)arg;

    // initialise the thread info. struct
    temp_thread->container     = thread_container;
    temp_thread->mstatus       = mstatus;
    temp_thread->mepc          = (uint32_t const)thread;
    temp_thread->stack_base_ed = thread_stack_base_ed;

    vmutex32_wait_for_lock(&new_thread_mutex, VMUTEX32_STATE_LOCKED);
    while (new_thread != VMEM32_NULL);
    new_thread = temp_thread;
    vmutex32_unlock(&new_thread_mutex);

    return temp_thread;
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

    active_thread = active_thread->next;

    // insert the new thread (if any) into the ring
    if (new_thread != VMEM32_NULL) {
        new_thread->next = active_thread->next;
        active_thread->next = new_thread;
        new_thread->previous = active_thread;
        new_thread->next->previous = new_thread;
        new_thread = VMEM32_NULL;
    }

    // remove the finished thread (if any) from the ring
    if (finished_thread != VMEM32_NULL && dead_thread == VMEM32_NULL) {
        dead_thread = finished_thread;
        finished_thread = VMEM32_NULL;
        dead_thread->previous->next = dead_thread->next;
        dead_thread->next->previous = dead_thread->previous;
    }

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
