#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char** argv) {
    makefile* mf = parse_makefile(argv[argc - 1]);

    int flag;
    while ((flag = getopt(argc, argv, "Bfs:"))) {
        switch (flag) {
        case 'B':
            /* code */
            break;

        case 'f':
            break;

        case 's':
            break;

        default:
            usage();
            break;
        }
    }

    return 0;
}