#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int fd = open("fil", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("open");
        exit(1);
    }

    if (dup2(fd, STDOUT_FILENO) == -1) {
        perror("dup2");
        close(fd);
        exit(1);
    }

    printf("Ey wallah!");

    close(fd);
    exit(0);
}
