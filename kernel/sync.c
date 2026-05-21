#include "sync.h"
#include "sched.h"
#include "task.h"

void sem_init(Semaphore *s, int initial, int max) {
    s->count = initial;
    s->max   = max;
    s->wn    = 0;
}

void sem_wait(Semaphore *s) {
    while (s->count == 0) {
        /* Park current task in the waiter list and yield. */
        Task *t = task_current();
        if (t && s->wn < 8) s->waiters[s->wn++] = t->id;
        t->state = TASK_BLOCKED;
        sched_yield();
    }
    s->count--;
}

void sem_post(Semaphore *s) {
    if (s->count < s->max) s->count++;
    /* Wake oldest waiter if any. */
    if (s->wn > 0) {
        int wid = s->waiters[0];
        for (int i = 1; i < s->wn; i++) s->waiters[i - 1] = s->waiters[i];
        s->wn--;
        if (tasks[wid].state == TASK_BLOCKED)
            tasks[wid].state = TASK_READY;
    }
}
