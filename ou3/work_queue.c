/**
 * @file work_queue.c
 * @brief Implementation of a thread-safe work queue for string paths.
 */

#include "work_queue.h"
#include <errno.h>

#define INITIAL_CAPACITY 1024

/**
 * @struct work_queue
 * @brief Internal structure representing the work queue.
 */
struct work_queue {
    char** paths; // Circular buffer of path strings
    int front; // Index of first element
    int rear; // Index of next free slot
    int size; // Current number of elements
    int capacity; // Buffer capacity
    int outstanding; // Number of unfinished tasks
    pthread_mutex_t mutex; // Protects all queue fields
    pthread_cond_t cond; // Signals queue state changes
};

/**
 * @brief Frees remaining paths in the queue (helper function).
 */
static void free_remaining_paths(work_queue_t* queue) {
    for (int i = 0; i < queue->size; i++) {
        free(queue->paths[(queue->front + i) % queue->capacity]);
    }
    free(queue->paths);
}

work_queue_t* work_queue_create(void) {
    work_queue_t* queue = malloc(sizeof(work_queue_t));
    if (!queue) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    queue->capacity = INITIAL_CAPACITY;
    queue->paths = malloc(sizeof(char*) * queue->capacity);
    if (!queue->paths) {
        perror("malloc");
        free(queue);
        exit(EXIT_FAILURE);
    }

    queue->front = queue->rear = queue->size = 0;
    queue->outstanding = 0;

    if (pthread_mutex_init(&queue->mutex, NULL) != 0) {
        perror("pthread_mutex_init");
        free(queue->paths);
        free(queue);
        exit(EXIT_FAILURE);
    }

    if (pthread_cond_init(&queue->cond, NULL) != 0) {
        perror("pthread_cond_init");
        pthread_mutex_destroy(&queue->mutex);
        free(queue->paths);
        free(queue);
        exit(EXIT_FAILURE);
    }

    return queue;
}

void work_queue_push(work_queue_t* queue, const char* path) {
    pthread_mutex_lock(&queue->mutex);

    // Expand buffer if full
    if (queue->size == queue->capacity) {
        int new_capacity = queue->capacity * 2;
        char** new_paths = malloc(sizeof(char*) * new_capacity);
        if (!new_paths) {
            perror("malloc");
            pthread_mutex_unlock(&queue->mutex);
            exit(EXIT_FAILURE);
        }

        // Copy elements in order to new buffer
        for (int i = 0; i < queue->size; i++) {
            new_paths[i] = queue->paths[(queue->front + i) % queue->capacity];
        }

        free(queue->paths);
        queue->paths = new_paths;
        queue->capacity = new_capacity;
        queue->front = 0;
        queue->rear = queue->size;
    }

    // Add new path
    queue->paths[queue->rear] = strdup(path);
    if (!queue->paths[queue->rear]) {
        perror("strdup");
        pthread_mutex_unlock(&queue->mutex);
        exit(EXIT_FAILURE);
    }

    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->size++;
    queue->outstanding++;

    pthread_cond_signal(&queue->cond);
    pthread_mutex_unlock(&queue->mutex);
}

char* work_queue_pop(work_queue_t* queue) {
    pthread_mutex_lock(&queue->mutex);

    // Wait for work or termination
    while (queue->size == 0) {
        if (queue->outstanding == 0) {
            pthread_cond_broadcast(&queue->cond);
            pthread_mutex_unlock(&queue->mutex);
            return NULL;
        }
        pthread_cond_wait(&queue->cond, &queue->mutex);
    }

    char* path = queue->paths[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size--;

    pthread_mutex_unlock(&queue->mutex);
    return path;
}

void work_queue_task_done(work_queue_t* queue) {
    pthread_mutex_lock(&queue->mutex);
    queue->outstanding--;

    if (queue->outstanding == 0) {
        pthread_cond_broadcast(&queue->cond);
    }

    pthread_mutex_unlock(&queue->mutex);
}

void work_queue_destroy(work_queue_t* queue) {
    pthread_mutex_destroy(&queue->mutex);
    pthread_cond_destroy(&queue->cond);
    free_remaining_paths(queue);
    free(queue);
}
