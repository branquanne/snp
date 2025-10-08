#ifndef WORK_QUEUE_H
#define WORK_QUEUE_H

#include "utils.h"

typedef struct work_queue work_queue_t;
work_queue_t* work_queue_create(void);
void work_queue_push(work_queue_t* queue, const char* path);
char* work_queue_pop(work_queue_t* queue);
void work_queue_task_done(work_queue_t* queue);
void work_queue_destroy(work_queue_t* queue);
bool work_queue_is_empty(work_queue_t* queue);
void free_pq(work_queue_t* queue);

#endif // !WORK_QUEUE_H
