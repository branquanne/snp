/**
 * @file dirsize.c
 * @brief Directory size calculation with optional parallel traversal.
 */

#include "dirsize.h"
#include "work_queue.h"
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/* --- INTERNAL --- */

/**
 * @struct thread_args_t
 * @brief Arguments passed to worker threads for parallel traversal.
 */
typedef struct {
    work_queue_t *queue;          /**< Pointer to the shared work queue */
    size_t *total_size;           /**< Pointer to the shared total size */
    pthread_mutex_t *size_mutex;  /**< Mutex for updating total size */
    int *had_access_error;        /**< Pointer to error flag */
    pthread_mutex_t *error_mutex; /**< Mutex for updating error flag */
} thread_args_t;

/**
 * @brief Frees resources and destroys mutexes.
 * @param threads Array of thread handles.
 * @param args Array of thread arguments.
 * @param size_mutex Mutex for total size.
 * @param error_mutex Mutex for error flag.
 * @param queue Work queue.
 */
static void free_and_destroy_mutex(pthread_t *threads, thread_args_t *args, pthread_mutex_t *size_mutex,
                                   pthread_mutex_t *error_mutex, work_queue_t *queue) {
    free(threads);
    free(args);
    pthread_mutex_destroy(size_mutex);
    pthread_mutex_destroy(error_mutex);
    queue_destroy(queue);
}

/**
 * @brief Reports a file/directory access error in a thread-safe way.
 */
static void report_access_error(const char *path, int *had_error, pthread_mutex_t *error_mutex) {
    perror(path);
    if (error_mutex) {
        pthread_mutex_lock(error_mutex);
        *had_error = 1;
        pthread_mutex_unlock(error_mutex);
    } else {
        *had_error = 1;
    }
}

/* --- EXTERNAL --- */

/**
 * @brief Worker thread function for parallel directory traversal.
 * Each worker repeatedly pops a path from the shared work queue.
 * For each path, it checks if it is a file or directory:
 *   - If a file, it adds its size to a local counter.
 *   - If a directory, it opens the directory and pushes all entries onto the queue.
 * Access errors are reported and flagged. When the queue is empty, the worker
 * adds its local size to the shared total size in a thread-safe way and exits.
 */
static void *worker_func(void *arg) {
    thread_args_t *args = (thread_args_t *)arg;
    size_t local_size = 0;

    while (1) {
        char *path = queue_pop(args->queue);
        if (!path) {
            break;
        }

        struct stat file_stat;
        if (lstat(path, &file_stat) == -1) {
            report_access_error(path, args->had_access_error, args->error_mutex);
            free(path);
            queue_task_done(args->queue);
            continue;
        }

        local_size += file_stat.st_blocks;

        if (S_ISDIR(file_stat.st_mode)) {
            DIR *dir = opendir(path);
            if (!dir) {
                report_access_error(path, args->had_access_error, args->error_mutex);
                free(path);
                queue_task_done(args->queue);
                continue;
            }

            struct dirent *entry;
            while ((entry = readdir(dir)) != NULL) {
                if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                    continue;
                }

                char full_path[PATH_MAX];
                int ret = snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
                if (ret >= (int)sizeof(full_path)) {
                    fprintf(stderr, "Filepath too long: %s/%s\n", path, entry->d_name);
                    report_access_error("", args->had_access_error, args->error_mutex);
                    continue;
                }
                queue_push(args->queue, full_path);
            }

            if (closedir(dir) == -1) {
                report_access_error(path, args->had_access_error, args->error_mutex);
            }
        }

        free(path);
        queue_task_done(args->queue);
    }

    pthread_mutex_lock(args->size_mutex);
    *(args->total_size) += local_size;
    pthread_mutex_unlock(args->size_mutex);

    return NULL;
}

/**
 * @brief Recursively calculates the disk usage of a path (single-threaded).
 * This function starts at the given path and checks if it is a file or directory:
 *   - If a file, it returns its size.
 *   - If a directory, it opens the directory and recursively calls itself for each entry.
 * The sizes of all files and subdirectories are accumulated.
 * Any access errors are reported and flagged.
 */
void get_size(const char *path, size_t *result, int *had_access_error) {
    struct stat file_stat;
    if (lstat(path, &file_stat) == -1) {
        report_access_error(path, had_access_error, NULL);
        *result = 0;
        return;
    }

    size_t total_size = file_stat.st_blocks;

    if (!S_ISDIR(file_stat.st_mode)) {
        *result = total_size;
        return;
    }

    DIR *dir = opendir(path);
    if (!dir) {
        report_access_error(path, had_access_error, NULL);
        *result = total_size;
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char full_path[PATH_MAX];
        int ret = snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        if (ret >= (int)sizeof(full_path)) {
            fprintf(stderr, "Filepath too long: %s/%s\n", path, entry->d_name);
            *had_access_error = 1;
            continue;
        }

        size_t child_size = 0;
        get_size(full_path, &child_size, had_access_error);
        total_size += child_size;
    }

    if (closedir(dir) == -1) {
        report_access_error(path, had_access_error, NULL);
    }

    *result = total_size;
}

/**
 * @brief Calculates the disk usage of a path using multiple threads.
 * This function sets up a work queue and pushes the initial path onto it.
 * It initializes mutexes and creates the specified number of worker threads.
 * Each thread processes files and directories from the queue in parallel.
 * After all threads finish, it cleans up resources and returns the total size.
 * Any access errors encountered are reported and flagged.
 */
void get_size_parallel(const char *path, int num_threads, size_t *result, int *had_access_error) {
    work_queue_t *queue = queue_create();
    queue_push(queue, path);

    size_t total_size = 0;
    pthread_mutex_t size_mutex, error_mutex;

    if (pthread_mutex_init(&size_mutex, NULL) != 0) {
        perror("pthread_mutex_init");
        queue_destroy(queue);
        exit(EXIT_FAILURE);
    }

    if (pthread_mutex_init(&error_mutex, NULL) != 0) {
        perror("pthread_mutex_init");
        pthread_mutex_destroy(&size_mutex);
        queue_destroy(queue);
        exit(EXIT_FAILURE);
    }

    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    thread_args_t *args = malloc(num_threads * sizeof(thread_args_t));

    if (!threads || !args) {
        perror("malloc");
        free_and_destroy_mutex(threads, args, &size_mutex, &error_mutex, queue);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < num_threads; i++) {
        args[i].queue = queue;
        args[i].total_size = &total_size;
        args[i].size_mutex = &size_mutex;
        args[i].had_access_error = had_access_error;
        args[i].error_mutex = &error_mutex;

        int ret = pthread_create(&threads[i], NULL, worker_func, &args[i]);
        if (ret != 0) {
            fprintf(stderr, "pthread_create: %s\n", strerror(ret));
            free_and_destroy_mutex(threads, args, &size_mutex, &error_mutex, queue);
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    free_and_destroy_mutex(threads, args, &size_mutex, &error_mutex, queue);

    *result = total_size;
}
