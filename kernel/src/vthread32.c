// TODO - catch thread exit return value

#include <vthread32.h>

#include <stdint.h>
#include <vmem32.h>
#include <vmutex32.h>
#include <syscall.h>

#define VTHREAD32_CONTEXT_SZW   30

struct thread_info {
    thread_id_t thread_id;
    void *container;
    uint32_t mstatus;
    uint32_t mepc;
    uint32_t const *stack_base_fd;
    thread_id_t join_to_id;
    uint32_t volatile wait_for_join;
    //
    uint32_t rtn_val;
    //
    struct thread_info *previous;
    struct thread_info *next;
};

thread_info_t dummy;

vmutex32_t      thread_ring_mutex;
unsigned        thread_ring_size;
thread_handle_t thread_ring;

thread_handle_t active_thread;

vmutex32_t      new_thread_mutex;
thread_handle_t volatile new_thread;

vmutex32_t      finished_thread_mutex;
thread_handle_t volatile finished_thread;

vmutex32_t      dead_thread_mutex;
thread_handle_t volatile dead_thread;

vmutex32_t  thread_id_mutex;
thread_id_t thread_id;

//#include <uart.h> // TODO - remove

static thread_id_t const vthread32_generate_id() {
    thread_id_t new_thread_id;

    vmutex32_wait_for_lock(&thread_id_mutex, VMUTEX32_STATE_LOCKED);

    new_thread_id = thread_id;
    ++thread_id;

    vmutex32_unlock(&thread_id_mutex);

    return new_thread_id;
}
static void vthread32_return_handler(void *const rtn_val) {
    uint32_t form[2];

    form[0] = SYSCALL_VTHREAD_FINISHED;
    form[1] = (uint32_t const)rtn_val;

    //printf("In return_handler (%u).\n", rtn_val);
    __asm__ volatile ("mv a0, %0; ecall" :: "r"(&form) : "a0", "memory");

    __builtin_unreachable();
}
static void *const vthread32_cleanup_deamon(void *const arg) {
    //printf("Starting cleanup deamon-----------.\n");
    while (1) {
        while (dead_thread == NULL);
        //printf("Running cleanup on thread 0x%X.\n", (uint32_t const)dead_thread);
        vmem32_free(dead_thread->container);
        vmem32_free(dead_thread);
        dead_thread = NULL;
    }

    __builtin_unreachable();
}
/* TODO - implement this
int const vthread32_kill(thread_handle_t const thread) {
    vmutex32_wait_for_lock(&finished_thread_mutex, VMUTEX32_STATE_LOCKED);
    while (finished_thread != NULL);
    finished_thread = thread;
    vmutex32_unlock(&finished_thread_mutex);
}
*/
int const vthread32_init(void *const(*thread)(void *const),
                         void *const arg, unsigned const stack_szw,
                         unsigned const kernel_stack_szw) {
    uint32_t *kernel_stack_base_fd;
    uint32_t *thread_stack_base_fd;
    void *thread_container;

    kernel_stack_base_fd = vmem32_alloc(kernel_stack_szw << 2)
                         + kernel_stack_szw;
    if (kernel_stack_base_fd == NULL) {
        return 1;
    }
    //printf("Kernel stack = 0x%x\n", kernel_stack_base_fd);

    thread_ring = vmem32_alloc(sizeof(thread_info_t));
    if (thread_ring == NULL) {
        vmem32_free(kernel_stack_base_fd);
        return 2;
    }
    //printf("thread_ring = 0x%x\n", thread_ring);

    thread_stack_base_fd = (uint32_t *const)(thread_container = vmem32_alloc(stack_szw << 2))
                         + stack_szw - VTHREAD32_CONTEXT_SZW;
    if (thread_stack_base_fd == NULL) {
        vmem32_free(kernel_stack_base_fd);
        vmem32_free(thread_ring);
        return 3;
    }

    __asm__ volatile ("csrw mscratch, %0" :: "r"(kernel_stack_base_fd) : "memory");

    thread_stack_base_fd[0] = (uint32_t const)vthread32_return_handler;
    thread_stack_base_fd[8] = (uint32_t const)arg;

    vmutex32_init(&thread_id_mutex);
    thread_id = 1;

    // initialise the thread info. struct
    thread_ring->thread_id     = vthread32_generate_id();
    thread_ring->container     = thread_container;
    thread_ring->mstatus       = 0x1880;
    thread_ring->mepc          = (uint32_t const)thread;
    thread_ring->stack_base_fd = thread_stack_base_fd;
    thread_ring->join_to_id    = 0;
    thread_ring->wait_for_join = 0;
    //
    thread_ring->previous = thread_ring;
    thread_ring->next     = thread_ring;

    thread_ring_size = 1;

    dummy.next = thread_ring;
    active_thread = &dummy;
    //printf("In switch, active = 0x%X, ", active_thread);

    vmutex32_init(&thread_ring_mutex);
    vmutex32_init(&new_thread_mutex);
    new_thread = NULL;
    vmutex32_init(&finished_thread_mutex);
    finished_thread = NULL;
    vmutex32_init(&dead_thread_mutex);
    dead_thread = NULL;

    vthread32_create(vthread32_cleanup_deamon, NULL, 1024u, 0x1880);

    return 0;
}
thread_id_t const vthread32_get_active(void) {
    return active_thread->thread_id;
}
unsigned const vthread32_get_all(thread_handle_t **const list) {
    thread_handle_t handle;
    unsigned thread_count;
    unsigned i;

    vmutex32_wait_for_lock(&thread_ring_mutex, VMUTEX32_STATE_LOCKED);

    handle = thread_ring->next;
    for (thread_count = 1; handle != thread_ring; ++thread_count) {
        handle = handle->next;
    }

    *list = (thread_handle_t *const)vmem32_alloc(thread_count * sizeof(thread_handle_t));

    handle = thread_ring;
    for (i = 0; i < thread_count; ++i) {
        (*list)[i] = handle;
        handle = handle->next;
    }

    vmutex32_unlock(&thread_ring_mutex);

    return thread_count;
}
thread_id_t const vthread32_create_raw(void *const(*thread)(void *const), void *const arg,
                                       void *const thread_container, uint32_t *const stack_base_fd,
                                       uint32_t const mstatus) {
    uint32_t *thread_stack_base_fd;
    thread_handle_t temp_thread;
    thread_id_t temp_thread_id;

    temp_thread = vmem32_alloc(sizeof(thread_info_t));
    if (temp_thread == NULL) {
        return 0;
    }
    //printf("temp_thread = 0x%x\n", temp_thread);

    thread_stack_base_fd = stack_base_fd - VTHREAD32_CONTEXT_SZW;

    thread_stack_base_fd[0] = (uint32_t const)vthread32_return_handler;
    thread_stack_base_fd[8] = (uint32_t const)arg;

    temp_thread_id = vthread32_generate_id();

    // initialise the thread info. struct
    temp_thread->thread_id     = temp_thread_id;
    temp_thread->container     = thread_container;
    temp_thread->mstatus       = mstatus;
    temp_thread->mepc          = (uint32_t const)thread;
    temp_thread->stack_base_fd = thread_stack_base_fd;
    temp_thread->join_to_id    = 0;
    temp_thread->wait_for_join = 0;

    vmutex32_wait_for_lock(&new_thread_mutex, VMUTEX32_STATE_LOCKED);
    while (new_thread != NULL);
    new_thread = temp_thread;
    vmutex32_unlock(&new_thread_mutex);

    return temp_thread_id;
}
thread_id_t const vthread32_create(void *const(*thread)(void *const), void *const arg,
                                   unsigned const stack_szw, uint32_t const mstatus) {
    uint32_t *stack_base_fd;
    thread_id_t temp_thread;
    void *thread_container;

    stack_base_fd = (uint32_t *const)(thread_container = vmem32_alloc(stack_szw << 2))
                  + stack_szw;
    if (thread_container == NULL) {
        return 0;
    }

    temp_thread = vthread32_create_raw(thread, arg, thread_container, stack_base_fd, mstatus);
    if (temp_thread == 0) {
        vmem32_free(thread_container);
        return 0;
    }

    return temp_thread;
}
int const vthread32_join(thread_id_t const thread, void **const rtn_val_ptr) {
    int found = 1;

    vmutex32_wait_for_lock(&thread_ring_mutex, VMUTEX32_STATE_LOCKED);

    thread_handle_t thread_iterator;
    for (thread_iterator = thread_ring;
         thread_iterator->next != thread_ring && thread_iterator->thread_id != thread;
         thread_iterator = thread_iterator->next);

    if (thread_iterator->thread_id == thread) {
        thread_iterator->join_to_id = active_thread->thread_id;
    } else if (new_thread->thread_id == thread) {
        new_thread->join_to_id = active_thread->thread_id;
    } else {
        found = 0;
    }

    vmutex32_unlock(&thread_ring_mutex);

    if (!found) {
        return 1;
    }


    active_thread->wait_for_join = 1;
    while(active_thread->wait_for_join);

    return 0;
}
void vthread32_finished_handler(uint32_t const rtn_val) {
    //printf("In finished_handler (%u).\n", rtn_val);
    active_thread->rtn_val = rtn_val;

    vmutex32_wait_for_lock(&thread_ring_mutex, VMUTEX32_STATE_LOCKED);
    thread_handle_t thread_iterator;
    for (thread_iterator = thread_ring;
         thread_iterator->next != thread_ring && thread_iterator->thread_id != active_thread->join_to_id;
         thread_iterator = thread_iterator->next);

    if (thread_iterator->thread_id == active_thread->join_to_id) {
        thread_iterator->wait_for_join = 0;
    }
    vmutex32_unlock(&thread_ring_mutex);

    vmutex32_wait_for_lock(&finished_thread_mutex, VMUTEX32_STATE_LOCKED);
    while (finished_thread != NULL);
    finished_thread = active_thread;
    vmutex32_unlock(&finished_thread_mutex);

    while (1);
    __builtin_unreachable();
}
void vthread32_switch(void) {
    uint32_t mstatus;
    uint32_t mepc;
    uint32_t const *stack_base_fd;

    __asm__ volatile ("csrr %0, mstatus" : "=r"(mstatus) :: "memory");
    __asm__ volatile ("csrr %0, mepc" : "=r"(mepc) :: "memory");
    __asm__ volatile ("csrr %0, mscratch" : "=r"(stack_base_fd) :: "memory");
    active_thread->mstatus       = mstatus;
    active_thread->mepc          = mepc;
    active_thread->stack_base_fd = stack_base_fd;

    active_thread = active_thread->next;

    if (!vmutex32_is_locked(&thread_ring_mutex)) {
        // insert the new thread (if any) into the ring
        if (new_thread != NULL) {
            ++thread_ring_size;
            new_thread->next = active_thread->next;
            active_thread->next = new_thread;
            new_thread->previous = active_thread;
            new_thread->next->previous = new_thread;
            new_thread = NULL;
        }

        // remove the finished thread (if any) from the ring
        if (finished_thread != NULL && dead_thread == NULL) {
            --thread_ring_size;
            if (finished_thread == active_thread) {
                active_thread = active_thread->next;
            }
            if (finished_thread == thread_ring) {
                thread_ring = active_thread;
            }
            dead_thread = finished_thread;
            finished_thread = NULL;
            dead_thread->previous->next = dead_thread->next;
            dead_thread->next->previous = dead_thread->previous;
        }
    }

    // choose next thread if new active thread is waiting for join
    thread_handle_t thread_iterator;
    for (thread_iterator = active_thread;
         thread_iterator->wait_for_join != 0 && thread_iterator->next != active_thread;
         thread_iterator = thread_iterator->next);
    active_thread = thread_iterator;

    mstatus       = active_thread->mstatus;
    mepc          = active_thread->mepc;
    stack_base_fd = active_thread->stack_base_fd;
    __asm__ volatile ("csrw mstatus, %0" :: "r"(mstatus) : "memory");
    __asm__ volatile ("csrw mepc, %0" :: "r"(mepc) : "memory");
    __asm__ volatile ("csrw mscratch, %0" :: "r"(stack_base_fd) : "memory");
}
/*
    __asm__ volatile ("csrr %0, mstatus" : "=r"(mstatus) :: "memory");
    __asm__ volatile ("csrr %0, mepc" : "=r"(mepc) :: "memory");
    __asm__ volatile ("csrr %0, mscratch" : "=r"(stack_base_fd) :: "memory");
    printf("Switch start: mstatus=0x%x, mepc=0x%x, stack=0x%x\n", mstatus, mepc, stack_base_fd);
*/
