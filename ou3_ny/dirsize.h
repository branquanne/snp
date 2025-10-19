/**
 * @file dirsize.h
 * @brief Functions for calculating directory sizes
 */

#ifndef DIRSIZE_H
#define DIRSIZE_H

#include <stdbool.h>
#include <stddef.h>

/**
 * Calculate the size of a file or directory recursively (single-threaded)
 * @param path Path to file or directory
 * @return Size in bytes, or 0 on error
 */
size_t calculate_dir_size(const char* path);

/**
 * Process directory with optional multi-threading
 * @param path Path to file or directory
 * @param use_threads Whether to use multi-threading
 * @param num_threads Number of threads to use
 * @return Size in bytes, or 0 on error
 */
size_t process_directory(const char* path, bool use_threads, int num_threads);

#endif // DIRSIZE_H
