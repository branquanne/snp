#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv) {
    sleep(1);
    fprintf(stdout, "Testing\n");
    sleep(1);
    if (argc < 0) {
        perror("Arguments");
        exit(1);
    } else if (argc >= 0) {
        for (int i = 0; i < argc; i++) {
            fprintf(stdout, "%s\n", argv[i]);
        }
    }

    exit(0);
}