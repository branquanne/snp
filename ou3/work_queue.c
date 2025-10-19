/**
 * @file work_queue.c
 * @brief Implementation of a thread-safe work queue for string paths.
 *
 * This file provides functions for creating, managing, and destroying a thread-safe
 * work queue, which is used to coordinate producer and consumer threads in parallel
 * directory traversal.
 */

#include "work_queue.h"
#include <stdatomic.h>
#include <unistd.h> // For usleep

/**
 * @brief Frees the memory allocated for the work queue structure.
 *
 * @param queue Pointer to the work queue to free.
 */
static void free_pq(work_queue_t* queue);

/**
 * @struct work_queue
 * @brief Internal structure representing the work queue.
 *
 * This structure holds the queue buffer, synchronization primitives, and thread counters.
 */
struct work_queue {
    char** paths;
    size_t capacity;
    size_t size;
    size_t front;

    pthread_mutex_t mutex;
    pthread_cond_t condition;

    int active_threads;
    int total_threads;
    bool terminate;
    atomic_int outstanding; /* tracks pushed items not yet finished */
};

/**
 * @brief Creates and initializes a new work queue.
 *
 * Allocates memory for the queue and its buffer, and initializes synchronization primitives.
 *
 * @return Pointer to the new work queue, or NULL on failure.
 */
work_queue_t* work_queue_create(void) {
    work_queue_t* queue = malloc(sizeof(work_queue_t));
    if (!queue) {
        fprintf(stderr, "Could not allocate memory for queue\n");
        return NULL;
    }

    queue->capacity = 1024;
    queue->paths = malloc(queue->capacity * sizeof(char*));
    if (!queue->paths) {
        fprintf(stderr, "Could not allocate memory for paths\n");
        free(queue);
        return NULL;
    }

    queue->size = 0;
    queue->front = 0;

    if (pthread_mutex_init(&queue->mutex, NULL) != 0) {
        perror("queue");
        free_pq(queue);
        return NULL;
    }

    if (pthread_cond_init(&queue->condition, NULL) != 0) {
        perror("queue");
        pthread_mutex_destroy(&queue->mutex);
        free_pq(queue);
        return NULL;
    }

    queue->active_threads = 0;
    queue->total_threads = 0;
    queue->terminate = false;
    atomic_init(&queue->outstanding, 0);

    return queue;
}

/**
 * @brief Pushes a new path onto the work queue.
 *
 * Expands the buffer if necessary and signals waiting threads.
 *
 * @param queue Pointer to the work queue.
 * @param path String path to add (copied internally).
 */
void work_queue_push(work_queue_t* queue, const char* path) {
    if (!queue || !path)
        return;

    pthread_mutex_lock(&queue->mutex);

    int retries = 0;
    while (queue->size >= queue->capacity && retries < 10) {
        const size_t temp_capacity = queue->capacity * 2;
        if (temp_capacity < queue->capacity) { // Overflow check
            fprintf(stderr, "Capacity overflow\n");
            pthread_mutex_unlock(&queue->mutex);
            return;
        }
        char** temp = malloc(temp_capacity * sizeof(char*));
        if (!temp) {
            retries++;
            usleep(1000 * retries); // Exponential backoff
            continue;
        }
        // Copy elements in correct order
        for (size_t i = 0; i < queue->size; ++i) {
            temp[i] = queue->paths[(queue->front + i) % queue->capacity];
        }
        free(queue->paths);
        queue->paths = temp;
        queue->capacity = temp_capacity;
        queue->front = 0;
        break;
    }

    if (retries >= 10) {
        fprintf(stderr, "Failed to realloc after 10 retries, skipping push\n");
        pthread_mutex_unlock(&queue->mutex);
        return;
    }

    const size_t index = (queue->front + queue->size) % queue->capacity;
    char* dup = strdup(path);
    if (!dup) {
        perror("strdup");
        pthread_mutex_unlock(&queue->mutex);
        return;
    }
    queue->paths[index] = dup;
    queue->size++;
    atomic_fetch_add(&queue->outstanding, 1);

    pthread_cond_signal(&queue->condition);
    pthread_mutex_unlock(&queue->mutex);
}

/**
 * @brief Pops a path from the work queue for processing.
 *
 * Waits if the queue is empty and not terminating. Increments active thread count.
 *
 * @param queue Pointer to the work queue.
 * @return Dynamically allocated string path, or NULL if terminating.
 */
char* work_queue_pop(work_queue_t* queue) {
    if (!queue) {
        return NULL;
    }

    pthread_mutex_lock(&queue->mutex);

    while (queue->size == 0 && !queue->terminate) {
        pthread_cond_wait(&queue->condition, &queue->mutex);
    }

    if (queue->size == 0) {
        pthread_mutex_unlock(&queue->mutex);
        return NULL;
    }

    char* path = queue->paths[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size--;
    queue->active_threads++;

    pthread_mutex_unlock(&queue->mutex);

    return path;
}

/**
 * @brief Signals that a thread has finished processing a path.
 *
 * Decrements the active thread count and signals termination if no threads or items remain.
 *
 * @param queue Pointer to the work queue.
 */
void work_queue_task_done(work_queue_t* queue) {
    if (!queue)
        return;

    int prev = atomic_fetch_sub(&queue->outstanding, 1);

    pthread_mutex_lock(&queue->mutex);
    queue->active_threads--;

    if (prev - 1 == 0) {
        queue->terminate = true;
        pthread_cond_broadcast(&queue->condition);
    }

    pthread_mutex_unlock(&queue->mutex);
}

/**
 * @brief Checks if the work queue is empty.
 *
 * @param queue Pointer to the work queue.
 * @return true if empty, false otherwise.
 */
bool work_queue_is_empty(work_queue_t* queue) {
    if (!queue)
        return true;

    pthread_mutex_lock(&queue->mutex);

    bool is_empty = (queue->size == 0);
    pthread_mutex_unlock(&queue->mutex);

    return is_empty;
}

/**
 * @brief Destroys the work queue and frees all associated memory.
 *
 * Frees all buffered paths, destroys synchronization primitives, and releases the queue.
 *
 * @param queue Pointer to the work queue.
 */
void work_queue_destroy(work_queue_t* queue) {
    if (!queue) {
        return;
    }

    pthread_mutex_lock(&queue->mutex);

    for (size_t i = 0; i < queue->size; i++) {
        size_t index = (queue->front + i) % queue->capacity;
        free(queue->paths[index]);
    }

    free(queue->paths);

    pthread_mutex_unlock(&queue->mutex);
    pthread_mutex_destroy(&queue->mutex);
    pthread_cond_destroy(&queue->condition);

    free(queue);
}

/**
 * @brief Frees the memory allocated for the work queue structure (helper).
 *
 * @param queue Pointer to the work queue to free.
 */
static void free_pq(work_queue_t* queue) {
    if (!queue)
        return;
    if (queue->paths)
        free(queue->paths);
    free(queue);
}
