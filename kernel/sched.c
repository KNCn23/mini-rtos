#include "sched.h"
#include "uart.h"
#include "task.h"

static SchedPolicy policy = SCHED_ROUND_ROBIN;
static int         current_id = -1;
static Context     scheduler_ctx;
static u64         tick_count = 0;

u64 timer_ms_now(void);

void sched_init(SchedPolicy p) {
    policy = p;
    task_table_init();
}

Task *task_current(void) {
    return current_id >= 0 ? &tasks[current_id] : NULL;
}

static int pick_next(void) {
    if (policy == SCHED_ROUND_ROBIN) {
        int start = current_id < 0 ? 0 : current_id;
        for (int step = 1; step <= n_tasks; step++) {
            int id = (start + step) % n_tasks;
            if (tasks[id].state == TASK_READY) return id;
        }
        /* If only the current task is runnable, keep running it. */
        if (current_id >= 0 && tasks[current_id].state == TASK_READY)
            return current_id;
        return -1;
    } else {
        int best = -1;
        int best_prio = 999;
        for (int i = 0; i < n_tasks; i++) {
            if (tasks[i].state == TASK_READY &&
                tasks[i].priority < best_prio) {
                best_prio = tasks[i].priority;
                best = i;
            }
        }
        return best;
    }
}

static void wake_sleepers(void) {
    u64 now = timer_ms_now();
    for (int i = 0; i < n_tasks; i++) {
        if (tasks[i].state == TASK_BLOCKED &&
            tasks[i].wake_at_ms > 0 &&
            tasks[i].wake_at_ms <= now) {
            tasks[i].state      = TASK_READY;
            tasks[i].wake_at_ms = 0;
        }
    }
}

static void switch_to(int next_id) {
    Task *prev = current_id >= 0 ? &tasks[current_id] : NULL;
    Task *next = &tasks[next_id];
    next->state = TASK_READY;     /* will become RUNNING after switch */
    next->ticks_ran++;
    int prev_id = current_id;
    current_id = next_id;
    if (prev) context_switch(&prev->ctx, &next->ctx);
    else      context_switch(&scheduler_ctx, &next->ctx);
    (void)prev_id;
}

void sched_yield(void) {
    /* Demote the currently running task to READY so it can be re-picked
     * alongside other ready tasks. */
    if (current_id >= 0 && tasks[current_id].state == TASK_READY) {
        /* nothing to do */
    }
    wake_sleepers();

    int next = pick_next();
    if (next < 0)                  return;          /* nothing else ready */
    if (next == current_id)        return;          /* still us           */
    switch_to(next);
}

void sched_sleep_ms(u64 ms) {
    Task *t = task_current();
    if (!t) return;
    t->wake_at_ms = timer_ms_now() + ms;
    t->state      = TASK_BLOCKED;

    /* Find another task to run. */
    wake_sleepers();
    int next = pick_next();
    if (next < 0 || next == current_id) {
        /* No one else is ready — busy-wait politely until we (or someone)
         * becomes runnable. wfe lowers power; cycle counter keeps ticking. */
        while (1) {
            asm volatile("yield");
            wake_sleepers();
            next = pick_next();
            if (next >= 0) break;
        }
    }
    if (next != current_id) switch_to(next);
}

void sched_tick(void) {
    tick_count++;
    wake_sleepers();
    int next = pick_next();
    if (next >= 0 && next != current_id) switch_to(next);
}

void sched_start(void) {
    wake_sleepers();
    int first = pick_next();
    if (first < 0) { uart_puts("[sched] no runnable tasks\n"); return; }
    switch_to(first);
    uart_puts("[sched] all tasks finished\n");
}

void sched_stats(void) {
    static const char *st_names[] = {"READY  ","RUNNING","BLOCKED","DEAD   "};
    uart_puts("\n── Task table ──\n");
    uart_puts("ID  Name     Prio  State    Ticks\n");
    for (int i = 0; i < n_tasks; i++) {
        Task *t = &tasks[i];
        uart_print_dec(t->id);    uart_puts("   ");
        uart_puts(t->name);       uart_puts("   ");
        uart_print_dec(t->priority); uart_puts("    ");
        uart_puts(st_names[t->state]); uart_puts("  ");
        uart_print_dec(t->ticks_ran); uart_putc('\n');
    }
    uart_puts("Total scheduler ticks: "); uart_print_dec(tick_count);
    uart_puts("\n\n");
}
