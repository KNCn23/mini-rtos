#ifndef TASK_H
#define TASK_H
#include "types.h"

#define MAX_TASKS    8
#define STACK_BYTES  4096

typedef enum {
    TASK_READY,
    TASK_RUNNING,
    TASK_BLOCKED,
    TASK_DEAD,
} TaskState;

/* Callee-saved AArch64 registers + sp + pc.
 * Layout MUST match the assembly in context_switch.S exactly. */
typedef struct {
    u64 x19, x20, x21, x22;
    u64 x23, x24, x25, x26;
    u64 x27, x28;
    u64 fp;       /* x29 */
    u64 lr;       /* x30 — return address */
    u64 sp;
} Context;

typedef struct {
    Context     ctx;
    TaskState   state;
    int         id;
    int         priority;     /* 0 = highest, 7 = lowest */
    u64         ticks_ran;
    u64         wake_at_ms;
    u64         stack_base;
    const char *name;
} Task;

typedef void (*TaskEntry)(void);

int   task_create(TaskEntry entry, int priority, const char *name);
Task *task_current(void);
void  task_table_init(void);

extern Task tasks[MAX_TASKS];
extern int  n_tasks;

#endif
