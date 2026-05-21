#include "types.h"
#include "uart.h"
#include "timer.h"
#include "sched.h"
#include "sync.h"

static Semaphore print_sem;

static void task_blinker(void) {
    int n = 0;
    while (1) {
        sem_wait(&print_sem);
        uart_puts("  [blinker] tick #"); uart_print_dec(++n);
        uart_puts("  (t="); uart_print_dec(timer_ms_now()); uart_puts(" ms)\n");
        sem_post(&print_sem);
        sched_sleep_ms(500);
    }
}

static void task_worker(void) {
    int batch = 0;
    while (1) {
        /* Simulate a unit of work, then yield politely. */
        u64 sum = 0;
        for (u64 i = 0; i < 200000; i++) sum += i * 3;
        (void)sum;
        batch++;
        if ((batch % 4) == 0) {
            sem_wait(&print_sem);
            uart_puts("  [worker]  batch #"); uart_print_dec(batch);
            uart_puts("  (t="); uart_print_dec(timer_ms_now()); uart_puts(" ms)\n");
            sem_post(&print_sem);
        }
        sched_sleep_ms(100);
    }
}

static void task_monitor(void) {
    while (1) {
        sched_sleep_ms(3000);
        sem_wait(&print_sem);
        sched_stats();
        sem_post(&print_sem);
    }
}

void kmain(void) {
    uart_init();
    timer_init();

    uart_puts("\nmini-rtos — AArch64 cooperative+preemptive kernel\n");
    uart_puts("══════════════════════════════════════════════════\n");

    sched_init(SCHED_PRIORITY);
    sem_init(&print_sem, 1, 1);

    /* Lower priority number = higher priority. */
    task_create(task_monitor, /*prio=*/1, "monitor");
    task_create(task_blinker, /*prio=*/2, "blinker");
    task_create(task_worker,  /*prio=*/3, "worker ");

    uart_puts("[boot] 3 tasks created — starting scheduler\n\n");
    sched_start();

    while (1) { timer_poll(); sched_yield(); }
}
