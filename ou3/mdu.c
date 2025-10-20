/**
 * @file mdu.c
 * @brief Main program for multi-threaded disk usage calculation.
 *
 * This program calculates the disk usage (in 512-byte blocks) of specified
 * files or directories. It supports parallel traversal using multiple threads.
 */

#include "dirsize.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* --- INTERNAL --- */

/**
 * @brief Prints usage information and exits.
 */
static void print_usage(void) {
    fprintf(stderr, "Usage: mdu [-j number_of_threads] file ...\n");
    exit(EXIT_FAILURE);
}

/**
 * @brief Parses the number of threads from command-line arguments.
 *
 * @param argc Argument count
 * @param argv Argument vector
 * @return Number of threads to use (default 1)
 */
static int get_thread_count(int argc, char **argv) {
    int num_threads = 1;
    int opt;

    while ((opt = getopt(argc, argv, "j:")) != -1) {
        switch (opt) {
        case 'j':
            num_threads = atoi(optarg);
            if (num_threads < 1) {
                fprintf(stderr, "Number of threads must be greater than 0\n");
                print_usage();
            }
            break;
        default:
            print_usage();
        }
    }
    return num_threads;
}

static void get_and_print_disk_usage(int argc, char **argv, int num_threads,
                                     int *had_access_error) {
    for (int i = optind; i < argc; i++) {
        size_t total_size = 0;
        int file_had_error = 0;

        if (num_threads > 1) {
            get_size_parallel(argv[i], num_threads, &total_size,
                              &file_had_error);
        } else {
            get_size(argv[i], &total_size, &file_had_error);
        }

        printf("%zu\t%s\n", total_size, argv[i]);

        if (file_had_error) {
            *had_access_error = 1;
        }
    }
}

/**
 * @brief Main entry point.
 *
 * Processes command-line arguments and calculates disk usage for each specified
 * file.
 */
int main(int argc, char **argv) {
    int num_threads = get_thread_count(argc, argv);

    if (optind >= argc) {
        print_usage();
    }

    int had_access_error = 0;
    get_and_print_disk_usage(argc, argv, num_threads, &had_access_error);

    return had_access_error ? EXIT_FAILURE : EXIT_SUCCESS;
}
