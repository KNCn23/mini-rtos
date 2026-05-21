#include "sched.h"
#include "uart.h"
#include "task.h"

static SchedPolicy policy = SCHED_ROUND_ROBIN;
static int         current_id = -1;
static Context     scheduler_ctx;
static u64         tick_count = 0;

/* Forward decl from timer driver */
u64 timer_ms_now(void);

void sched_init(SchedPolicy p) {
    policy = p;
    task_table_init();
}

Task *task_current(void) {
    return current_id >= 0 ? &tasks[current_id] : NULL;
}

/* Pick the next runnable task. Returns -1 if nothing to run. */
static int pick_next(void) {
    int best = -1;
    if (policy == SCHED_ROUND_ROBIN) {
        for (int i = 1; i <= n_tasks; i++) {
            int id = (current_id + i) % n_tasks;
            if (tasks[id].state == TASK_READY ||
                tasks[id].state == TASK_RUNNING)
                return id;
        }
    } else {
        int best_prio = 999;
        for (int i = 0; i < n_tasks; i++) {
            if ((tasks[i].state == TASK_READY ||
                 tasks[i].state == TASK_RUNNING) &&
                tasks[i].priority < best_prio) {
                best_prio = tasks[i].priority;
                best = i;
            }
        }
        return best;
    }
    return best;
}

/* Wake any tasks whose sleep has expired */
static void wake_sleepers(void) {
    u64 now = timer_ms_now();
    for (int i = 0; i < n_tasks; i++) {
        if (tasks[i].state == TASK_BLOCKED &&
            tasks[i].wake_at_ms <= now) {
            tasks[i].state = TASK_READY;
            tasks[i].wake_at_ms = 0;
        }
    }
}

static void switch_to(int next_id) {
    Task *prev = current_id >= 0 ? &tasks[current_id] : NULL;
    Task *next = &tasks[next_id];
    if (prev && prev->state == TASK_RUNNING) prev->state = TASK_READY;
    next->state = TASK_RUNNING;
    next->ticks_ran++;
    current_id = next_id;
    if (prev) context_switch(&prev->ctx, &next->ctx);
    else      context_switch(&scheduler_ctx, &next->ctx);
}

void sched_yield(void) {
    wake_sleepers();
    int next = pick_next();
    if (next < 0 || next == current_id) return;
    switch_to(next);
}

void sched_sleep_ms(u64 ms) {
    Task *t = task_current();
    if (!t) return;
    t->wake_at_ms = timer_ms_now() + ms;
    t->state      = TASK_BLOCKED;
    sched_yield();
}

void sched_tick(void) {
    tick_count++;
    wake_sleepers();
    /* Preemption: every tick, yield to the scheduler. */
    int next = pick_next();
    if (next >= 0 && next != current_id) switch_to(next);
}

void sched_start(void) {
    wake_sleepers();
    int first = pick_next();
    if (first < 0) {
        uart_puts("[sched] no runnable tasks\n");
        return;
    }
    switch_to(first);
    /* Scheduler context resumes here only if all tasks exit. */
    uart_puts("[sched] all tasks finished\n");
}

void sched_stats(void) {
    uart_puts("\n── Task table ──\n");
    uart_puts("ID  Name             Prio  State    Ticks\n");
    for (int i = 0; i < n_tasks; i++) {
        Task *t = &tasks[i];
        uart_print_dec(t->id);   uart_puts("   ");
        uart_puts(t->name);      uart_puts("\t   ");
        uart_print_dec(t->priority); uart_puts("    ");
        const char *st = "?";
        switch (t->state) {
            case TASK_READY:   st = "READY  "; break;
            case TASK_RUNNING: st = "RUNNING"; break;
            case TASK_BLOCKED: st = "BLOCKED"; break;
            case TASK_DEAD:    st = "DEAD   "; break;
        }
        uart_puts(st); uart_puts("  ");
        uart_print_dec(t->ticks_ran); uart_putc('\n');
    }
    uart_puts("Total ticks: "); uart_print_dec(tick_count);
    uart_puts("\n\n");
}
