/**
 * @file work_queue.h
 * @brief Thread-safe work queue for parallel directory traversal.
 */

#ifndef WORK_QUEUE_H
#define WORK_QUEUE_H

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Opaque work queue type.
 */
typedef struct work_queue work_queue_t;

/**
 * @brief Creates a new work queue.
 * @return Pointer to the created queue, exits on failure.
 */
work_queue_t* work_queue_create(void);

/**
 * @brief Adds a path to the work queue.
 * @param queue Work queue
 * @param path Path string (will be copied)
 */
void work_queue_push(work_queue_t* queue, const char* path);

/**
 * @brief Retrieves a path from the work queue.
 * @param queue Work queue
 * @return Dynamically allocated path string, or NULL if done
 */
char* work_queue_pop(work_queue_t* queue);

/**
 * @brief Signals completion of a task.
 * @param queue Work queue
 */
void work_queue_task_done(work_queue_t* queue);

/**
 * @brief Destroys the work queue and frees resources.
 * @param queue Work queue
 */
void work_queue_destroy(work_queue_t* queue);

#endif // WORK_QUEUE_H
