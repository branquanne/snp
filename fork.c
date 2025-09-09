#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char** argv) {
    pid_t pid;

    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    } else if (pid == 0) {
        printf("Hello\n");
        if (execl("./test_prog", (char*)NULL) < 0) {
            perror("execl");
            exit(1);
        }
    } else {
        printf("Hello\n");
        printf("%d\n", getpid());
        wait(NULL);
        printf("Bye\n");
    }

    exit(0);
}