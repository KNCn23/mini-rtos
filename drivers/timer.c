#include "timer.h"
#include "sched.h"

static u64 boot_ticks;
static u64 next_tick_ms;
#define TICK_INTERVAL_MS 50

static inline u64 read_cntfrq(void) {
    u64 v; asm volatile("mrs %0, cntfrq_el0" : "=r"(v)); return v;
}
static inline u64 read_cntpct(void) {
    u64 v; asm volatile("mrs %0, cntpct_el0" : "=r"(v)); return v;
}

void timer_init(void) {
    boot_ticks   = read_cntpct();
    next_tick_ms = 0;
}

u64 timer_ms_now(void) {
    u64 freq = read_cntfrq();
    return ((read_cntpct() - boot_ticks) * 1000UL) / freq;
}

void timer_poll(void) {
    u64 now = timer_ms_now();
    if (now >= next_tick_ms) {
        next_tick_ms = now + TICK_INTERVAL_MS;
        sched_tick();
    }
}
