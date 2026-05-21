#ifndef SCHED_H
#define SCHED_H

#include "task.h"

typedef enum {
    SCHED_ROUND_ROBIN,
    SCHED_PRIORITY,
} SchedPolicy;

void sched_init(SchedPolicy policy);
void sched_start(void);          /* never returns */
void sched_yield(void);          /* cooperative yield */
void sched_sleep_ms(u64 ms);
void sched_tick(void);           /* called from the timer ISR/poll */
void sched_stats(void);

/* Implemented in context_switch.S */
extern void context_switch(Context *old, Context *new);

#endif
