#ifndef DIRSIZE_H
#define DIRSIZE_H

#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

size_t calculate_dir_size(const char* path);
size_t process_directory(const char* path, int num_threads);

#endif // !DIRSIZE_H
