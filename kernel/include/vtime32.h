#ifndef VTIME32_H
#define VTIME32_H

#include <stdint.h>

typedef struct vtime32 {
    unsigned days;
    unsigned hours;
    unsigned minutes;
    unsigned seconds;
} vtime32_t;

typedef struct vtime32 vtime32_t;

typedef struct vtime32_context {
    uint32_t dev_base_addr;
    uint32_t time_step_us;
    uint32_t volatile micro_seconds;
    uint32_t volatile remainder_us;
    //
    vtime32_t volatile time;
} vtime32_context_t;

typedef uint32_t vtime32_timer_t;

void vtime32_init(vtime32_context_t *const timer, uint32_t const timer_base_addr,
                  unsigned const clk_freq_hz, unsigned const period_us);
void vtime32_isr(vtime32_context_t *const timer);
void vtime32_wait_ms(vtime32_context_t const *const timer, unsigned const delay);
void vtime32_set_timer_ms(vtime32_context_t const *const timer,
                          vtime32_timer_t *const clock, unsigned const start_ms);
int const vtime32_is_expired(vtime32_context_t const *const timer, vtime32_timer_t const *const clock);

#endif
