#ifndef DIRSIZE_H
#define DIRSIZE_H

#include <stddef.h>

void get_size(const char *path, size_t *result, int *had_access_error);
void get_size_parallel(const char *path, int num_threads, size_t *result,
                       int *had_access_error);

#endif // !DIRSIZE_H
