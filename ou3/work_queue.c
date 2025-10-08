#include "work_queue.h"

struct work_queue {
    char** paths;
    size_t capacity;
    size_t size;
    size_t front;

    pthread_mutex_t mutex;
    pthread_mutex_t condition;

    int active_threads;
    int total_threads;
    bool terminate;
};

work_queue_t* work_queue_create(void) {
    work_queue_t* queue = malloc(sizeof(work_queue_t));
    if (!queue) {
        fprintf(stderr, "Could not allocate memory for queue");
        return NULL;
    }

    queue->capacity = 16;
    queue->paths = malloc(queue->capacity * sizeof(char*));
    if (!queue->paths) {
        fprintf(stderr, "Could not allocate memory for paths");
        return NULL;
    }

    queue->size = 0;
    queue->front = 0;

    if (pthread_mutex_init(&queue->mutex, NULL) != 0) {
        perror(queue);
        free_pq(queue);
        return NULL;
    }

    if (pthread_cond_init(&queue->condition, NULL) != 0) {
        perror(queue);
        pthread_mutex_destroy(&queue->mutex);
        free_pq(queue);
    }
}

void free_pq(work_queue_t* queue) {
    free(queue->paths);
    free(queue);
}