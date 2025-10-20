#ifndef DIRSIZE_H
#define DIRSIZE_H

#include <stddef.h>

void calculate_dir_size(const char* path, size_t* result, int* had_access_error);
void process_directory(const char* path, int num_threads, size_t* result, int* had_access_error);

#endif // !DIRSIZE_H
