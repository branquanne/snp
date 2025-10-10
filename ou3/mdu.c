#include "dirsize.h"

static void usage() {
    fprintf(stderr, "Usage: mdu [-j number_of_threads] file ...\n");
    exit(EXIT_FAILURE);
}

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

        size_t total_size = calculate_dir_size(filename);
        size_t blocks = (total_size + 511) / 512;

        printf("%s\t%d\n", filename, num_threads);
    }

    return EXIT_SUCCESS;
}
