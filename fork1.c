#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main(void) {
    pid_t pid1 = -1;
    pid_t pid2 = -1;
    pid_t pid3 = -1;
    pid_t pid4 = -1;

    printf(" %6s\t%6s\t%6s\t%6s\n", "PID", "PPID", "pid1", "pid2");
    printf("Utskrift 1 %6d\t%6d\t%6d\t%6d\n", getpid(), getppid(), pid1, pid2);
    pid1 = fork();
    pid2 = fork();
    pid3 = fork();
    pid4 = fork();

    printf("Utskrift 2 %6d\t%6d\t%6d\t%6d\n", getpid(), getppid(), pid1, pid2);
    exit(0);
}