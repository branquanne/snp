/**
 * @file dirsize.c
 * @brief Implements directory size calculation using a thread-safe work queue.
 *
 * This file provides functions to recursively calculate the size of directories,
 * optionally using multiple threads for parallel traversal.
 */

#include "dirsize.h"
#include "work_queue.h"
#include <stddef.h>
#include <string.h>

/**
 * @struct thread_args
 * @brief Arguments passed to worker threads.
 */
struct thread_args {
    work_queue_t* queue;
    size_t* total_size;
    pthread_mutex_t* size_mutex;
};

/**
 * @brief Worker thread function for processing paths from the work queue.
 *
 * Pops paths from the queue, calculates their size, and pushes subdirectories
 * back onto the queue for further processing. Accumulates the size in a local
 * variable and updates the shared total_size at the end.
 *
 * @param arg Pointer to thread_args structure.
 * @return NULL
 */
static void* worker_func(void* arg) {
    struct thread_args* args = (struct thread_args*)arg;
    work_queue_t* queue = args->queue;
    size_t local_size = 0;

    while (1) {
        char* path = work_queue_pop(queue);
        if (!path) {
            break;
        }

        struct stat file_stat;
        if (lstat(path, &file_stat) == -1) {
            perror(path);
            free(path);
            work_queue_task_done(queue);
            continue;
        }

        if (S_ISREG(file_stat.st_mode) || S_ISDIR(file_stat.st_mode)) {
            local_size += file_stat.st_blocks;
        }

        if (S_ISDIR(file_stat.st_mode)) {
            DIR* dir = opendir(path);
            if (!dir) {
                perror(path);
                free(path);
                work_queue_task_done(queue);
                continue;
            }
            struct dirent* entry;
            while ((entry = readdir(dir)) != NULL) {
                if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                    continue;
                }
                char full_path[PATH_MAX];
                int ret = snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
                if (ret >= (int)sizeof(full_path)) {
                    fprintf(stderr, "Filepath too long!\n");
                    continue;
                }
                work_queue_push(queue, full_path);
            }
            closedir(dir);
        }
        free(path);
        work_queue_task_done(queue);
    }

    pthread_mutex_lock(args->size_mutex);
    *(args->total_size) += local_size;
    pthread_mutex_unlock(args->size_mutex);

    return NULL;
}

/**
 * @brief Recursively calculates the size of a directory (single-threaded).
 *
 * Traverses the directory tree and sums the sizes of all regular files.
 *
 * @param path Path to the directory or file.
 * @return Total size in bytes.
 */
size_t calculate_dir_size(const char* path) {
    struct stat file_stat;
    if (lstat(path, &file_stat) == -1) {
        perror(path);
        return 0;
    }

    size_t total_size = 0;
    if (S_ISREG(file_stat.st_mode) || S_ISDIR(file_stat.st_mode)) {
        total_size += file_stat.st_blocks;
    }

    if (!S_ISDIR(file_stat.st_mode)) {
        return total_size;
    }

    DIR* dir = opendir(path);
    if (!dir) {
        perror(path);
        return total_size;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char full_path[PATH_MAX];
        int ret = snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        if (ret >= (int)sizeof(full_path)) {
            fprintf(stderr, "Filepath too long!\n");
            continue;
        }

        total_size += calculate_dir_size(full_path);
    }

    if (closedir(dir) == -1) {
        perror(path);
    }

    return total_size;
}

/**
 * @brief Calculates the size of a directory, optionally using multiple threads.
 *
 * Initializes a work queue and spawns worker threads to process directory entries
 * in parallel. The total size is accumulated and returned.
 *
 * @param path Path to the directory.
 * @param use_threads Unused parameter (for compatibility).
 * @param num_threads Number of worker threads to use.
 * @return Total size in bytes.
 */
size_t process_directory(const char* path, bool use_threads, int num_threads) {
    (void)use_threads;
    work_queue_t* queue = work_queue_create();
    if (!queue) {
        fprintf(stderr, "Failed to create work queue!\n");
        return 0;
    }

    work_queue_push(queue, path);

    size_t total_size = 0;
    pthread_mutex_t size_mutex;
    pthread_mutex_init(&size_mutex, NULL);

    pthread_t threads[num_threads];
    struct thread_args args = { queue, &total_size, &size_mutex };

    for (int i = 0; i < num_threads; i++) {
        int ret = pthread_create(&threads[i], NULL, worker_func, &args);
        if (ret != 0) {
            fprintf(stderr, "pthread_create: %s\n", strerror(ret));
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&size_mutex);
    work_queue_destroy(queue);

    return total_size;
}
