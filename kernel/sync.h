#ifndef SYNC_H
#define SYNC_H
#include "types.h"

typedef struct {
    int count;          /* current resource count                 */
    int max;            /* max value (binary semaphore: max == 1) */
    int waiters[8];     /* simple FIFO of task IDs                */
    int wn;
} Semaphore;

void sem_init(Semaphore *s, int initial, int max);
void sem_wait(Semaphore *s);
void sem_post(Semaphore *s);

#endif
