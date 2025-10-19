/**
 * @file work_queue.h
 * @brief Thread-safe work queue for parallel directory traversal
 */

#ifndef WORK_QUEUE_H
#define WORK_QUEUE_H

#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct work_queue work_queue_t;

/**
 * Create and initialize a new work queue
 * @return Pointer to newly created work queue, or NULL on error
 */
work_queue_t* work_queue_create(void);

/**
 * Add a directory path to the work queue
 * @param queue The work queue
 * @param path The path to add (will be duplicated)
 */
void work_queue_push(work_queue_t* queue, const char* path);

/**
 * Remove and return the next path from the work queue
 * @param queue The work queue
 * @return Path string (caller must free), or NULL if shutting down
 */
char* work_queue_pop(work_queue_t* queue);

/**
 * Notify the queue that a task has been completed
 * @param queue The work queue
 */
void work_queue_task_done(work_queue_t* queue);

/**
 * Check if the work queue is empty
 * @param queue The work queue
 * @return true if empty, false otherwise
 */
bool work_queue_is_empty(work_queue_t* queue);

/**
 * Destroy the work queue and free all resources
 * @param queue The work queue to destroy
 */
void work_queue_destroy(work_queue_t* queue);

#endif // WORK_QUEUE_H
