#include <vtime32.h>

#include <common.h>

#define TIMER_TIME_OFFSET           0
#define TIMER_MAX_OFFSET            4
#define TIMER_CTRL_OFFSET           8
#define TIMER_INT_OFFSET            12
//
#define TIMER_CTRL_INT_EN_MASK      0x2
#define TIMER_CTRL_EN_MASK          0x1

static void vtime32_increment(vtime32_context_t *const timer) {
        ++timer->time.seconds;
        if (timer->time.seconds >= 60) {
            timer->time.seconds = 0;
            ++timer->time.minutes;
        }
        if (timer->time.minutes >= 60) {
            timer->time.minutes = 0;
            ++timer->time.hours;
        }
        if (timer->time.hours >= 24) {
            timer->time.hours = 0;
            ++timer->time.days;
        }
}
//
void vtime32_init(vtime32_context_t *const timer, uint32_t const timer_base_addr,
                  unsigned const clk_freq_hz, unsigned const period_us) {
    uint32_t const period_cycles = (uint32_t const)((clk_freq_hz * period_us) / 1000000);
    //uint32_t const period_cycles = 2500;

    timer->dev_base_addr = timer_base_addr;
    timer->time_step_us  = (uint32_t const)period_us;
    timer->micro_seconds = 0;
    timer->remainder_us  = 0;
    timer->time.days     = 0;
    timer->time.hours    = 0;
    timer->time.minutes  = 0;
    timer->time.seconds  = 0;
    //printf("cycles=%u, interval=%u\n", period_cycles, interval_us);
    write_reg(timer_base_addr + TIMER_MAX_OFFSET, period_cycles - 1);
    write_reg(timer_base_addr + TIMER_TIME_OFFSET, 0);
    write_reg(timer_base_addr + TIMER_INT_OFFSET, 0);
    write_reg(timer_base_addr + TIMER_CTRL_OFFSET, TIMER_CTRL_EN_MASK | TIMER_CTRL_INT_EN_MASK);
}
void vtime32_isr(vtime32_context_t *const timer) {
    write_reg(timer->dev_base_addr + TIMER_INT_OFFSET, 0);
    timer->micro_seconds += timer->time_step_us;

    timer->remainder_us += timer->time_step_us;
    if (timer->remainder_us >= 1000000) {
        timer->remainder_us -= 1000000;
        vtime32_increment(timer);
    }
}
void vtime32_wait_ms(vtime32_context_t const *const timer, unsigned const delay) {
    unsigned const start_time = timer->micro_seconds;

    while (timer->micro_seconds - start_time < (uint32_t const)delay * 1000);
}
void vtime32_set_timer_ms(vtime32_context_t const *const timer,
                          vtime32_timer_t *const clock, unsigned const start_ms) {
    *clock = timer->micro_seconds + (uint32_t const)(start_ms * 1000);
}
int const vtime32_is_expired(vtime32_context_t const *const timer, vtime32_timer_t const *const clock) {
    return (*clock - timer->micro_seconds) >> 31;
}
