#include <cqueue.h>

#define CQUEUE_PTR_MASK     ((CQUEUE_SZ << 1) - 1)

//#define CQUEUE_LOCK_WAIT    while ()

void cqueue_init(cqueue_t *const queue) {
    queue->rd_ptr = 0;
    queue->wr_ptr = 0;
    //vmutex32_init(&(queue->lock));
}
int volatile const cqueue_empty(cqueue_t const *const queue) {
    return queue->rd_ptr == queue->wr_ptr;
}
int volatile const cqueue_full(cqueue_t const *const queue) {
    return (queue->rd_ptr ^ queue->wr_ptr) == CQUEUE_SZ;
}
char const cqueue_getc(cqueue_t *const queue) {
    char c;
    c = queue->buffer[queue->rd_ptr & (CQUEUE_PTR_MASK >> 1)];
    queue->rd_ptr = (queue->rd_ptr + 1) & CQUEUE_PTR_MASK;
    return c;
}
int const cqueue_putc(cqueue_t *const queue, char const c) {
    queue->buffer[queue->wr_ptr & (CQUEUE_PTR_MASK >> 1)] = c;
    queue->wr_ptr = (queue->wr_ptr + 1) & CQUEUE_PTR_MASK;
    return 0;
}
