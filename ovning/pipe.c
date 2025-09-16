#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#define READ_END 0
#define WRITE_END 1

int main(int argc, char** argv) {
    int fd[2];
    char msg[] = "broooo";
    char buffer[100];
    pid_t pid;

    if (pipe(fd) == -1) {
        perror("pipe");
        exit(1);
    }

    if ((pid = fork()) == -1) {
        perror("fork");
        exit(1);
    } else if (pid == 0) {
        close(fd[READ_END]);
        if (write(fd[WRITE_END], msg, sizeof(msg)) == -1) {
            perror("write");
            exit(1);
        }
    } else {
        close(fd[WRITE_END]);
        wait(NULL);
        if (read(fd[READ_END], buffer, sizeof(buffer)) == -1) {
            perror("read");
            exit(1);
        }
    }

    printf("%s\n", buffer);

    close(fd[READ_END]);
    close(fd[WRITE_END]);

    exit(0);
}