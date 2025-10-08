#include "dirsize.h"
#include "work_queue.h"

struct thread_args {
    work_queue_t* queue;
    size_t* total_size;
    pthread_mutex_t* size_mutex;
};

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

        if (S_ISREG(file_stat.st_mode)) {
            local_size += file_stat.st_size;
        } else if (S_ISDIR(file_stat.st_mode)) {
            DIR* dir = opendir(path);
            if (!dir) {
                perror(path);
                free(path);
                work_queue_task_done(queue);
                continue;
            }
            struct dirent* entry;
            while (entry = readdir(dir) != NULL) {
                if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                    continue;
                }
                char full_path[PATH_MAX];
                int ret = snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
                if (ret >= (int)sizeof(full_path)) {
                    fprintf(stderr, "Filepath too long!\n");
                    continue;
                }
                work_queue_push(queue, path);
            }
            closedir(dir);
        }
        free(path);
        work_queue_task_done(queue);
    }

    pthread_mutex_lock(&args->size_mutex);
    *(args->total_size) += local_size;
    pthread_mutex_unlock(&args->size_mutex);

    return NULL;
}

size_t calculate_dir_size(const char* path) {
    struct stat file_stat;
    if (lstat(path, &file_stat) == -1) {
        perror(path);
        return 0;
    }

    if (S_ISREG(file_stat.st_mode)) {
        return file_stat.st_size;
    }

    if (!S_ISDIR(file_stat.st_mode)) {
        return 0;
    }

    // Open the directory for reading
    // Check if opendir failed and handle error appropriately
    DIR* dir = opendir(path);
    if (!dir) {
        perror(path);
    }

    // Initialize total size counter to accumulate directory contents
    size_t total_size = 0;
    struct dirent* entry;

    // Loop through each entry in the directory using readdir
    while (entry = readdir(dir) != NULL) {
        // Skip the current directory entry "."
        // Skip the parent directory entry ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Construct the full path by combining current path with entry name
        // Check if the constructed path exceeds maxidmum path length
        char full_path[PATH_MAX];
        int ret = snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        if (ret >= (int)sizeof(full_path)) {
            fprintf(stderr, "Filepath too long!\n");
            continue;
        }

        // Recursively call calculate_dir_size on the full path
        // Add the returned size to our total size accumulator
        total_size += calculate_dir_size(full_path);
    }
    // End of directory traversal loop

    // Close the directory and check for errors
    if (closedir(dir) == -1) {
        perror(path);
    }

    // Return the total accumulated size of all directory contents
    return total_size;
}

size_t process_directory(const char* path, bool use_threads, int num_threads) {
    if (!use_threads || num_threads <= 1) {
        return calculate_dir_size(path);
    }

    work_queue_t* queue = work_queue_create();
    if (!queue) {
        fprintf(stderr, "Failed to create work queue!\n");
        return NULL;
    }

    
}