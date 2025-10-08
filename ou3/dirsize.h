#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

size_t calculate_dir_size(const char* path);
size_t process_directory(const char* path, bool use_threads, int num_threads);