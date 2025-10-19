/**
 * @file mdu.c
 * @brief Main program file for mdu - disk usage calculator
 * @author Your Name
 * @date 2025-10-19
 */

#include "dirsize.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * Print usage information and exit
 */
static void usage(void) {
    fprintf(stderr, "Usage: mdu [-j antal_trådar] fil ...\n");
    exit(EXIT_FAILURE);
}

/**
 * Main function
 * @param argc Argument count
 * @param argv Argument vector
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on error
 */
int main(int argc, char** argv) {
    int num_threads = 1;

    // Parse command line arguments
    int flag;
    while ((flag = getopt(argc, argv, "j:")) != -1) {
        switch (flag) {
        case 'j':
            num_threads = atoi(optarg);
            if (num_threads < 1) {
                fprintf(stderr, "Number of threads must be at least 1\n");
                usage();
            }
            break;
        default:
            usage();
            break;
        }
    }

    // Check if at least one file/directory was specified
    if (optind >= argc) {
        fprintf(stderr, "No files specified\n");
        usage();
    }

    // Process each file/directory specified
    for (int i = optind; i < argc; i++) {
        char* filename = argv[i];

        // Calculate size using single-threaded or multi-threaded approach
        // The result is already in 512-byte blocks (from st_blocks)
        size_t blocks;
        if (num_threads > 1) {
            blocks = process_directory(filename, true, num_threads);
        } else {
            blocks = calculate_dir_size(filename);
        }

        // Output in format matching du -s -l -B512
        printf("%zu\t%s\n", blocks, filename);
    }

    return EXIT_SUCCESS;
}
