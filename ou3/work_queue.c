/**
 * @file work_queue.c
 * @brief Implementation of a thread-safe work queue for string paths.
 * @date 2025-11-19
 * @author Bran Mjöberg Quanne
 */

#include "work_queue.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>

/* --- INTERNAL --- */

/**
 * @struct work_queue
 * @brief Internal structure representing the work queue.
 */
struct work_queue {
    char **paths;          /**< Circular buffer of path strings */
    int front;             /**< Index of first element */
    int rear;              /**< Index of next free slot */
    int size;              /**< Current number of elements */
    int capacity;          /**< Buffer capacity */
    int outstanding;       /**< Number of unfinished tasks */
    pthread_mutex_t mutex; /**< Protects all queue fields */
    pthread_cond_t cond;   /**< Signals queue state changes */
};

/**
 * @brief Frees remaining paths in the queue.
 */
static void free_remaining_paths(work_queue_t *queue) {
    for (int i = 0; i < queue->size; i++) {
        free(queue->paths[(queue->front + i) % queue->capacity]);
    }
    free(queue->paths);
}

/* --- EXTERNAL --- */

/**
 * @brief Creates a new work queue.
 *
 * Allocates and initializes a thread-safe work queue for managing path strings.
 * Sets up the circular buffer, mutex, and condition variable.
 * If any allocation or initialization fails, the program exits with an error.
 * Returns a pointer to the newly created queue.
 */
work_queue_t *queue_create(void) {
    work_queue_t *queue = malloc(sizeof(work_queue_t));
    if (!queue) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    queue->capacity = 1024;
    queue->paths = malloc(sizeof(char *) * queue->capacity);
    if (!queue->paths) {
        perror("malloc");
        free(queue);
        exit(EXIT_FAILURE);
    }

    queue->front = queue->rear = queue->size = 0;
    queue->outstanding = 0;

    int errnum = pthread_mutex_init(&queue->mutex, NULL);
    if (errnum != 0) {
        fprintf(stderr, "pthread_mutex_init: %s\n", strerror(errnum));
        free(queue->paths);
        free(queue);
        exit(EXIT_FAILURE);
    }

    errnum = pthread_cond_init(&queue->cond, NULL);
    if (errnum != 0) {
        fprintf(stderr, "pthread_cond_init: %s\n", strerror(errnum));
        errnum = pthread_mutex_destroy(&queue->mutex);
        if (errnum != 0) {
            fprintf(stderr, "pthread_mutex_destroy: %s\n", strerror(errnum));
            exit(EXIT_FAILURE);
        }
        free(queue->paths);
        free(queue);
        exit(EXIT_FAILURE);
    }

    return queue;
}

/**
 * @brief Adds a path to the work queue.
 *
 * Copies the given path string and adds it to the queue.
 * If the queue is full, it dynamically expands the buffer to accommodate more paths.
 * Signals waiting threads that new work is available.
 */
void queue_push(work_queue_t *queue, const char *path) {
    safe_lock(&queue->mutex);

    if (queue->size == queue->capacity) {
        int new_capacity = queue->capacity * 2;
        char **new_paths = malloc(sizeof(char *) * new_capacity);
        if (!new_paths) {
            perror("malloc");
            safe_unlock(&queue->mutex);
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < queue->size; i++) {
            new_paths[i] = queue->paths[(queue->front + i) % queue->capacity];
        }

        free(queue->paths);
        queue->paths = new_paths;
        queue->capacity = new_capacity;
        queue->front = 0;
        queue->rear = queue->size;
    }

    queue->paths[queue->rear] = strdup(path);
    if (!queue->paths[queue->rear]) {
        perror("strdup");
        safe_unlock(&queue->mutex);
        exit(EXIT_FAILURE);
    }

    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->size++;
    queue->outstanding++;

    int errnum = pthread_cond_signal(&queue->cond);
    if (errnum != 0) {
        fprintf(stderr, "pthread_cond_signal: %s\n", strerror(errnum));
        exit(EXIT_FAILURE);
    }
    safe_unlock(&queue->mutex);
}

/**
 * @brief Retrieves a path from the work queue.
 *
 * Waits if the queue is empty but there are outstanding tasks.
 * Returns a dynamically allocated path string for processing, or NULL if all tasks are done.
 */
char *queue_pop(work_queue_t *queue) {
    safe_lock(&queue->mutex);

    while (queue->size == 0) {
        if (queue->outstanding == 0) {
            int errnum = pthread_cond_broadcast(&queue->cond);
            if (errnum != 0) {
                fprintf(stderr, "pthread_cond_broadcast: %s\n", strerror(errnum));
                exit(EXIT_FAILURE);
            }
            safe_unlock(&queue->mutex);
            return NULL;
        }
        int errnum = pthread_cond_wait(&queue->cond, &queue->mutex);
        if (errnum != 0) {
            fprintf(stderr, "pthread_cond_wait: %s\n", strerror(errnum));
            exit(EXIT_FAILURE);
        }
    }

    char *path = queue->paths[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size--;

    safe_unlock(&queue->mutex);
    return path;
}

/**
 * @brief Signals completion of a task.
 *
 * Decrements the count of outstanding tasks in the queue.
 * If all tasks are completed, it wakes up any threads waiting for work.
 */
void queue_task_done(work_queue_t *queue) {
    safe_lock(&queue->mutex);
    queue->outstanding--;

    if (queue->outstanding == 0) {
        int errnum = pthread_cond_broadcast(&queue->cond);
        if (errnum != 0) {
            fprintf(stderr, "pthread_cond_broadcast: %s\n", strerror(errnum));
            exit(EXIT_FAILURE);
        }
    }

    safe_unlock(&queue->mutex);
}

/**
 * @brief Destroys the work queue and frees resources.
 */
void queue_destroy(work_queue_t *queue) {
    int errnum = pthread_mutex_destroy(&queue->mutex);
    if (errnum != 0) {
        fprintf(stderr, "pthread_mutex_destroy: %s\n", strerror(errnum));
        exit(EXIT_FAILURE);
    }

    errnum = pthread_cond_destroy(&queue->cond);
    if (errnum != 0) {
        fprintf(stderr, "pthread_cond_destroy: %s\n", strerror(errnum));
        exit(EXIT_FAILURE);
    }
    free_remaining_paths(queue);
    free(queue);
}

/**
 * @brief Safely lock a pthread mutex.
 *
 * This helper wraps pthread_mutex_lock and checks the return value.
 * On failure it prints the error number and terminates the process.
 */
void safe_lock(pthread_mutex_t *m) {
    int errnum = pthread_mutex_lock(m);
    if (errnum != 0) {
        fprintf(stderr, "pthread_mutex_lock: %s\n", strerror(errnum));
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Safely unlock a pthread mutex.
 *
 * This helper wraps pthread_mutex_unlock and checks the return value.
 * On failure it prints the errornumber and terminates the process.
 */
void safe_unlock(pthread_mutex_t *m) {
    int errnum = pthread_mutex_unlock(m);
    if (errnum != 0) {
        fprintf(stderr, "pthread_mutex_unlock: %s\n", strerror(errnum));
        exit(EXIT_FAILURE);
    }
}
