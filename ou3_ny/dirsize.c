/**
 * @file dirsize.c
 * @brief Implementation of directory size calculation functions
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

/**
 * Thread arguments structure
 */
struct thread_args {
    work_queue_t* queue;
    size_t* total_size;
    pthread_mutex_t* size_mutex;
};

/**
 * Worker thread function for parallel directory traversal
 */
static void* worker_func(void* arg) {
    struct thread_args* args = (struct thread_args*)arg;
    work_queue_t* queue = args->queue;
    size_t local_size = 0;

    while (1) {
        // Get next path from queue
        char* path = work_queue_pop(queue);
        if (!path) {
            break; // Shutdown signal received
        }

        // Get file information
        struct stat file_stat;
        if (lstat(path, &file_stat) == -1) {
            perror(path);
            free(path);
            work_queue_task_done(queue);
            continue;
        }

        // Add the disk blocks used (st_blocks is in 512-byte blocks)
        local_size += file_stat.st_blocks;

        // If it's a directory, traverse it
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
                // Skip . and ..
                if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                    continue;
                }

                // Build full path
                char full_path[PATH_MAX];
                int ret = snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
                if (ret >= (int)sizeof(full_path)) {
                    fprintf(stderr, "Path too long: %s/%s\n", path, entry->d_name);
                    continue;
                }

                // Add to work queue
                work_queue_push(queue, full_path);
            }

            if (closedir(dir) == -1) {
                perror("closedir");
            }
        }

        free(path);
        work_queue_task_done(queue);
    }

    // Add local size to global total
    pthread_mutex_lock(args->size_mutex);
    *(args->total_size) += local_size;
    pthread_mutex_unlock(args->size_mutex);

    return NULL;
}

/**
 * Calculate directory size using single-threaded recursive approach
 */
size_t calculate_dir_size(const char* path) {
    struct stat file_stat;

    // Get file information
    if (lstat(path, &file_stat) == -1) {
        perror(path);
        return 0;
    }

    // Start with the blocks used by this file/directory itself
    size_t total_size = file_stat.st_blocks;

    // If it's not a directory, just return the blocks used
    if (!S_ISDIR(file_stat.st_mode)) {
        return total_size;
    }

    // It's a directory - traverse it
    DIR* dir = opendir(path);
    if (!dir) {
        perror(path);
        return total_size; // Return what we have so far
    }

    struct dirent* entry;

    while ((entry = readdir(dir)) != NULL) {
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Build full path
        char full_path[PATH_MAX];
        int ret = snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        if (ret >= (int)sizeof(full_path)) {
            fprintf(stderr, "Path too long: %s/%s\n", path, entry->d_name);
            continue;
        }

        // Recursively calculate size
        total_size += calculate_dir_size(full_path);
    }

    if (closedir(dir) == -1) {
        perror("closedir");
    }

    return total_size;
}

/**
 * Process directory with optional multi-threading
 */
size_t process_directory(const char* path, bool use_threads, int num_threads) {
    // Use single-threaded version if not using threads
    if (!use_threads || num_threads <= 1) {
        return calculate_dir_size(path);
    }

    // Create work queue
    work_queue_t* queue = work_queue_create();
    if (!queue) {
        fprintf(stderr, "Failed to create work queue\n");
        return 0;
    }

    // Add initial path to queue
    work_queue_push(queue, path);

    // Initialize shared data
    size_t total_size = 0;
    pthread_mutex_t size_mutex;
    int ret = pthread_mutex_init(&size_mutex, NULL);
    if (ret != 0) {
        fprintf(stderr, "pthread_mutex_init: %s\n", strerror(ret));
        work_queue_destroy(queue);
        return 0;
    }

    // Allocate thread array
    pthread_t* threads = malloc(num_threads * sizeof(pthread_t));
    if (!threads) {
        perror("malloc");
        work_queue_destroy(queue);
        pthread_mutex_destroy(&size_mutex);
        return 0;
    }

    // Prepare thread arguments
    struct thread_args args = {
        .queue = queue,
        .total_size = &total_size,
        .size_mutex = &size_mutex
    };

    // Create worker threads
    for (int i = 0; i < num_threads; i++) {
        ret = pthread_create(&threads[i], NULL, worker_func, &args);
        if (ret != 0) {
            fprintf(stderr, "pthread_create: %s\n", strerror(ret));
            // Clean up and return
            for (int j = 0; j < i; j++) {
                pthread_join(threads[j], NULL);
            }
            work_queue_destroy(queue);
            pthread_mutex_destroy(&size_mutex);
            free(threads);
            return 0;
        }
    }

    // Wait for all threads to complete
    for (int i = 0; i < num_threads; i++) {
        ret = pthread_join(threads[i], NULL);
        if (ret != 0) {
            fprintf(stderr, "pthread_join: %s\n", strerror(ret));
        }
    }

    // Clean up
    work_queue_destroy(queue);
    pthread_mutex_destroy(&size_mutex);
    free(threads);

    return total_size;
}
