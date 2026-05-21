#include "task.h"
#include "types.h"

Task tasks[MAX_TASKS];
int  n_tasks = 0;

/* 4 KB per task, statically allocated, 16-byte aligned. */
static u8 stacks[MAX_TASKS][STACK_BYTES]
    __attribute__((aligned(16)));

void task_table_init(void) {
    n_tasks = 0;
    for (int i = 0; i < MAX_TASKS; i++)
        tasks[i].state = TASK_DEAD;
}

int task_create(TaskEntry entry, int priority, const char *name) {
    if (n_tasks >= MAX_TASKS) return -1;
    int id = n_tasks++;
    Task *t = &tasks[id];

    t->id        = id;
    t->name      = name;
    t->state     = TASK_READY;
    t->priority  = priority;
    t->ticks_ran = 0;
    t->wake_at_ms = 0;
    t->stack_base = (u64)stacks[id];

    /* Stack grows down; start at the top, 16-byte aligned. */
    u64 sp = (u64)&stacks[id][STACK_BYTES];
    sp &= ~0xFUL;

    /* First time the task runs, the context-switch code restores
     * callee-saved regs and branches to lr.  We set lr to the entry
     * function; the rest stay zero. */
    t->ctx.sp = sp;
    t->ctx.lr = (u64)entry;
    t->ctx.fp = 0;
    return id;
}
