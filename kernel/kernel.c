#include "types.h"
#include "uart.h"
#include "timer.h"
#include "sched.h"
#include "sync.h"

static Semaphore print_sem;

/* ── Three demo tasks ─────────────────────────────────────────────────── */

static void task_blinker(void) {
    int n = 0;
    while (1) {
        sem_wait(&print_sem);
        uart_puts("  [blinker] tick #"); uart_print_dec(++n); uart_putc('\n');
        sem_post(&print_sem);
        sched_sleep_ms(500);
    }
}

static void task_counter(void) {
    u64 i = 0;
    while (1) {
        i++;
        if ((i % 100000) == 0) {
            sem_wait(&print_sem);
            uart_puts("  [counter] reached "); uart_print_dec(i); uart_putc('\n');
            sem_post(&print_sem);
            sched_yield();
        }
    }
}

static void task_monitor(void) {
    while (1) {
        sched_sleep_ms(2000);
        sem_wait(&print_sem);
        sched_stats();
        sem_post(&print_sem);
    }
}

/* ── Boot ──────────────────────────────────────────────────────────────── */

void kmain(void) {
    uart_init();
    timer_init();

    uart_puts("\nmini-rtos — AArch64 cooperative+preemptive kernel\n");
    uart_puts("══════════════════════════════════════════════════\n");

    sched_init(SCHED_PRIORITY);
    sem_init(&print_sem, 1, 1);

    task_create(task_blinker, /*prio=*/2, "blinker");
    task_create(task_counter, /*prio=*/3, "counter");
    task_create(task_monitor, /*prio=*/1, "monitor");

    uart_puts("[boot] 3 tasks created — starting scheduler\n\n");

    /* Bootstrap: first task is launched explicitly, then the timer ISR
     * (here a poll inside each context) drives preemption. */
    sched_start();

    /* Tick loop, in case sched returns. */
    while (1) {
        timer_poll();
        sched_yield();
    }
}
