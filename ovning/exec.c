#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv) {
    if (execl("/bin/ls", "ls", (char *)NULL) == -1) {
        perror("execl");
        exit(1);
    }
    exit(0);
}