/**
 * @file mdu.c
 * @brief Main program for multi-threaded disk usage calculation.
 *
 * This program calculates the disk usage (in 512-byte blocks) of specified
 * files or directories. It supports parallel traversal using multiple threads.
 *
 * Usage: mdu [-j number_of_threads] file ...
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
 * This function scans the command-line arguments for the '-j' option.
 * Returns the number of threads to use (default is 1 if not specified).
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
/**
 * @brief Calculates and prints disk usage for each specified file or directory.
 *
 * This function iterates over all file arguments provided on the command line.
 * For each file or directory:
 *   - If parallel mode is requested (num_threads > 1), it calls get_size_parallel.
 *   - Otherwise, it calls get_size for single-threaded calculation.
 * It prints the disk usage (in blocks) and the file name for each entry.
 * If any access errors occur, it sets the error flag.
 */
static void get_and_print_disk_usage(int argc, char **argv, int num_threads, int *had_access_error) {
    for (int i = optind; i < argc; i++) {
        size_t total_size = 0;
        int file_had_error = 0;

        if (num_threads > 1) {
            get_size_parallel(argv[i], num_threads, &total_size, &file_had_error);
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
 * This function processes command-line arguments, determines the number of threads,
 * and validates that at least one file or directory is specified.
 * It then calls get_and_print_disk_usage to calculate and display disk usage for each entry.
 * The program exits with a failure status if any access errors occurred, otherwise exits successfully.
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
