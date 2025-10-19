/**
 * @file mdu.c
 * @brief Main program for multi-threaded disk usage calculation.
 *
 * This program calculates the disk usage (in 512-byte blocks) of specified files or directories.
 * It supports parallel traversal using multiple threads.
 */

#include "dirsize.h"
#include <stddef.h>

/**
 * @brief Prints usage information and exits the program.
 *
 * This function displays the correct command-line usage and terminates the program.
 */
static void usage(void) {
    fprintf(stderr, "Usage: mdu [-j number_of_threads] file ...\n");
    exit(EXIT_FAILURE);
}

/**
 * @brief Entry point for the mdu program.
 *
 * Parses command-line arguments, determines the number of threads to use,
 * and calculates disk usage for each specified file or directory.
 *
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return EXIT_SUCCESS on success, exits on error.
 */
int main(int argc, char** argv) {

    int num_threads = 1;

    int flag;
    while ((flag = getopt(argc, argv, "j:")) != -1) {
        switch (flag) {
        case 'j':
            num_threads = atoi(optarg);
            if (num_threads < 2) {
                fprintf(stderr, "Number of specified threads must be greater than 1\n");
                usage();
            }
            break;
        default:
            usage();
            break;
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "No files specified");
        usage();
    }

    for (int i = optind; i < argc; i++) {
        char* filename = argv[i];
        size_t total_size = 0;

        if (num_threads > 1) {
            total_size = process_directory(filename,  num_threads);
        } else {
            total_size = calculate_dir_size(filename);
        }
        

        size_t blocks = total_size;
        printf("%zu\t%s\n", blocks, filename);
    }

    return EXIT_SUCCESS;
}
