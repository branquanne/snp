#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void) {
    pid_t pid;

    pid = fork();
    if (pid < 0) {
        perror("PID");
        exit(1);
    } else if (pid == 0) {
        sleep(3);
        if (execl("./argv", "hej", "yäni", "vgd?", NULL) < 0) {
            perror("EXEC");
            exit(1);
        }
    } else {
        sleep(1);
        printf("Parent signing off!\n");
        int status;
        waitpid(pid, &status, 0);
        printf("Child exited with status: %d\n", WEXITSTATUS(status));
        printf("Child terminated by signal %d\n", WTERMSIG(status));
    }

    exit(0);
}