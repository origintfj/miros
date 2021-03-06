#ifndef VTHREAD32_H
#define VTHREAD32_H

#include <stdint.h>

typedef struct thread_info thread_info_t;
typedef thread_info_t *thread_handle_t;

typedef uint64_t thread_id_t;

int const vthread32_init(void *const(*thread)(void *const),
                         void *const arg, unsigned const stack_szw,
                         unsigned const kernel_stack_szw);
thread_id_t const vthread32_get_active(void);
unsigned const vthread32_get_all(thread_handle_t **const list);
thread_id_t const vthread32_create_raw(void *const(*thread)(void *const), void *const arg,
                                       void *const thread_container, uint32_t *const stack_base_fd,
                                       uint32_t const mstatus);
thread_id_t const vthread32_create(void *const(*thread)(void *const), void *const arg,
                                   unsigned const stack_szw, uint32_t const mstatus);
int const vthread32_join(thread_id_t const thread, void **const rtn_val_ptr);
void vthread32_finished_handler(uint32_t const rtn_val);
void vthread32_switch(void);

#define VTHREAD32_HANDOFF   __asm__ volatile ("1: csrs mstatus, 0x8; j 1b" ::: "memory");

#endif
