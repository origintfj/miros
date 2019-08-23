#ifndef CQUEUE_H
#define CQUEUE_H

#define CQUEUE_SZX          10
#define CQUEUE_SZ           (1 << CQUEUE_SZX)

#include <vmutex32.h>

typedef struct {
    vmutex32_t lock;
    unsigned volatile rd_ptr;
    unsigned volatile wr_ptr;
    char volatile buffer[CQUEUE_SZ];
} cqueue_t;

void cqueue_init(cqueue_t *const queue);
int volatile const cqueue_empty(cqueue_t const *const queue);
int volatile const cqueue_full(cqueue_t const *const queue);
char const cqueue_getc(cqueue_t *const queue);
int const cqueue_putc(cqueue_t *const queue, char const c);

#endif
