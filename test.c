#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
static int g = 4;

int main(int arge, char* argv[]) {
    int var = 10;
    pid_t ret;
    while (--var > 4)
        if ((ret = fork()) < 0) {
            perror("fork error");
            exit(1);
        } else if (ret == 0) {
            g += 2;
            var--;
        } else {
            var -= 2;
            g++;
            if (waitpid(ret, NULL, 0) != ret) {
                perror("waitpid error");
                exit(1);
            }
        }
    printf("mypid = %d \t parentpid = %d \t ret = %d \t var = %d \t g = %d\n", getpid(), getppid(), ret, var, g);
    exit(0);
}
