#ifndef DIRSIZE_H
#define DIRSIZE_H

#include "utils.h"

size_t calculate_dir_size(const char* path);
size_t process_directory(const char* path, bool use_threads, int num_threads);

#endif // !DIRSIZE_H
