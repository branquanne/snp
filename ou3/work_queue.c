#include "work_queue.h"

void free_pq(work_queue_t* queue);

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
        perror("queue");
        free_pq(queue);
        return NULL;
    }

    if (pthread_cond_init(&queue->condition, NULL) != 0) {
        perror("queue");
        pthread_mutex_destroy(&queue->mutex);
        free_pq(queue);
    }

    queue->active_threads = 0;
    queue->total_threads = 0;
    queue->terminate = false;

    return queue;
}

void work_queue_push(work_queue_t* queue, const char* path) {
    pthread_mutex_lock(&queue->mutex);

    if (queue->size >= queue->capacity) {
        queue->capacity *= 2;
        queue->paths = realloc(queue->paths, queue->capacity * sizeof(char*));
    }

    size_t index = (queue->front + queue->size) % queue->capacity;
    queue->paths[index] = strdup(path);
    queue->size++;

    pthread_cond_signal(&queue->condition);

    pthread_mutex_unlock(&queue->mutex);
}

char* work_queue_pop(work_queue_t* queue) {
    pthread_mutex_lock(&queue->mutex);

    while (queue->size == 0 && !queue->terminate) {
        pthread_cond_wait(&queue->condition, &queue->mutex);
    }

    if (queue->size == 0 && !queue->terminate) {
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

void work_queue_task_done(work_queue_t* queue) {
    pthread_mutex_lock(&queue->mutex);

    queue->active_threads--;

    if (queue->active_threads == 0 && queue->size == 0) {
        queue->terminate = true;
        pthread_cond_broadcast(&queue->condition);
    }

    pthread_mutex_unlock(&queue->mutex);
}

bool work_queue_is_empty(work_queue_t* queue) {
    pthread_mutex_lock(&queue->mutex);

    bool is_empty = (queue->size == 0);
    pthread_mutex_unlock(&queue->mutex);

    return is_empty;
}

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

void free_pq(work_queue_t* queue) {
    free(queue->paths);
    free(queue);
}
