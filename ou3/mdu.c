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

/**
 * @brief Prints usage information and exits.
 */
static void usage(void) {
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
static int parse_num_threads(int argc, char** argv) {
    int num_threads = 1;
    int opt;

    while ((opt = getopt(argc, argv, "j:")) != -1) {
        switch (opt) {
        case 'j':
            num_threads = atoi(optarg);
            if (num_threads < 1) {
                fprintf(stderr, "Number of threads must be greater than 0\n");
                usage();
            }
            break;
        default:
            usage();
        }
    }
    return num_threads;
}

/**
 * @brief Main entry point.
 *
 * Processes command-line arguments and calculates disk usage for each specified file.
 */
int main(int argc, char** argv) {
    int num_threads = parse_num_threads(argc, argv);

    if (optind >= argc) {
        fprintf(stderr, "No files specified\n");
        usage();
    }

    int had_access_error = 0;

    for (int i = optind; i < argc; i++) {
        size_t total_size = 0;
        int file_had_error = 0;

        if (num_threads > 1) {
            process_directory(argv[i], num_threads, &total_size, &file_had_error);
        } else {
            calculate_dir_size(argv[i], &total_size, &file_had_error);
        }

        printf("%zu\t%s\n", total_size, argv[i]);

        if (file_had_error) {
            had_access_error = 1;
        }
    }

    return had_access_error ? EXIT_FAILURE : EXIT_SUCCESS;
}
