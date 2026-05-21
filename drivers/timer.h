#ifndef TIMER_H
#define TIMER_H
#include "types.h"
void timer_init(void);
u64  timer_ms_now(void);
void timer_poll(void);     /* call from main loop to fire scheduler ticks */
#endif
