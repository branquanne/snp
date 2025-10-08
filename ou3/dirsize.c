#include "dirsize.h"

size_t calculate_dir_size(const char* path) {
    struct stat file_stat;
    if (stat(path, &file_stat) == -1) {
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