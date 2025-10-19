/**
 * @file work_queue.c
 * @brief Implementation of thread-safe work queue
 */

#include "work_queue.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Work queue structure - circular buffer implementation
 */
struct work_queue {
    char** paths; // Dynamic array of path strings
    size_t capacity; // Total capacity of the array
    size_t size; // Current number of items
    size_t front; // Index of first item to be popped

    pthread_mutex_t mutex; // Mutex for thread synchronization
    pthread_cond_t cond; // Condition variable for signaling

    int active_threads; // Number of threads currently processing work
    int total_threads; // Total number of worker threads
    bool shutdown; // Flag to indicate shutdown
};

/**
 * Create and initialize a new work queue
 */
work_queue_t* work_queue_create(void) {
    work_queue_t* queue = malloc(sizeof(work_queue_t));
    if (!queue) {
        perror("malloc");
        return NULL;
    }

    // Initialize the paths array
    queue->capacity = 16;
    queue->paths = malloc(queue->capacity * sizeof(char*));
    if (!queue->paths) {
        perror("malloc");
        free(queue);
        return NULL;
    }

    queue->size = 0;
    queue->front = 0;

    // Initialize mutex
    int ret = pthread_mutex_init(&queue->mutex, NULL);
    if (ret != 0) {
        fprintf(stderr, "pthread_mutex_init: %s\n", strerror(ret));
        free(queue->paths);
        free(queue);
        return NULL;
    }

    // Initialize condition variable
    ret = pthread_cond_init(&queue->cond, NULL);
    if (ret != 0) {
        fprintf(stderr, "pthread_cond_init: %s\n", strerror(ret));
        pthread_mutex_destroy(&queue->mutex);
        free(queue->paths);
        free(queue);
        return NULL;
    }

    queue->active_threads = 0;
    queue->total_threads = 0;
    queue->shutdown = false;

    return queue;
}

/**
 * Add a new path to the work queue
 */
void work_queue_push(work_queue_t* queue, const char* path) {
    pthread_mutex_lock(&queue->mutex);

    // Resize if needed
    if (queue->size >= queue->capacity) {
        queue->capacity *= 2;
        char** new_paths = realloc(queue->paths, queue->capacity * sizeof(char*));
        if (!new_paths) {
            perror("realloc");
            pthread_mutex_unlock(&queue->mutex);
            return;
        }
        queue->paths = new_paths;
    }

    // Add path to the end of the queue
    size_t index = (queue->front + queue->size) % queue->capacity;
    queue->paths[index] = strdup(path);
    queue->size++;

    // Signal waiting threads that work is available
    pthread_cond_signal(&queue->cond);

    pthread_mutex_unlock(&queue->mutex);
}

/**
 * Remove and return the next path from the work queue
 */
char* work_queue_pop(work_queue_t* queue) {
    pthread_mutex_lock(&queue->mutex);

    // Wait while queue is empty and not shutting down
    while (queue->size == 0 && !queue->shutdown) {
        pthread_cond_wait(&queue->cond, &queue->mutex);
    }

    // Check if we should terminate
    if (queue->size == 0 && queue->shutdown) {
        pthread_mutex_unlock(&queue->mutex);
        return NULL;
    }

    // Remove path from front of queue
    char* path = queue->paths[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size--;

    // Mark thread as active
    queue->active_threads++;

    pthread_mutex_unlock(&queue->mutex);

    return path;
}

/**
 * Notify that a task has been completed
 */
void work_queue_task_done(work_queue_t* queue) {
    pthread_mutex_lock(&queue->mutex);

    queue->active_threads--;

    // If no active threads and no work left, initiate shutdown
    if (queue->active_threads == 0 && queue->size == 0) {
        queue->shutdown = true;
        pthread_cond_broadcast(&queue->cond);
    }

    pthread_mutex_unlock(&queue->mutex);
}

/**
 * Check if the work queue is empty
 */
bool work_queue_is_empty(work_queue_t* queue) {
    pthread_mutex_lock(&queue->mutex);
    bool empty = (queue->size == 0);
    pthread_mutex_unlock(&queue->mutex);
    return empty;
}

/**
 * Destroy the work queue and free all resources
 */
void work_queue_destroy(work_queue_t* queue) {
    if (!queue) {
        return;
    }

    pthread_mutex_lock(&queue->mutex);

    // Free all remaining paths
    for (size_t i = 0; i < queue->size; i++) {
        size_t index = (queue->front + i) % queue->capacity;
        free(queue->paths[index]);
    }

    free(queue->paths);

    pthread_mutex_unlock(&queue->mutex);

    pthread_mutex_destroy(&queue->mutex);
    pthread_cond_destroy(&queue->cond);

    free(queue);
}
