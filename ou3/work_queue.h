/**
 * @file work_queue.h
 * @brief Thread-safe work queue interface for enqueuing path strings.
 * @author Bran Mjöberg Quanne
 * @date 2025-11-19
 */
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
void safe_lock(pthread_mutex_t *m);
void safe_unlock(pthread_mutex_t *m);

#endif // WORK_QUEUE_H
