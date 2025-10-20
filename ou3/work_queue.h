#ifndef WORK_QUEUE_H
#define WORK_QUEUE_H

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct work_queue work_queue_t;
work_queue_t *queue_create(void);
void queue_push(work_queue_t *queue, const char *path);
char *queue_pop(work_queue_t *queue);
void queue_task_done(work_queue_t *queue);
void queue_destroy(work_queue_t *queue);

#endif // WORK_QUEUE_H
