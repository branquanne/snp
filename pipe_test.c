#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define READ_END 0
#define WRITE_END 1

int main(int argc, char **argv) {
    int parent_to_child_1[2];
    int child_1_to_child_2[2];
    int child_2_to_parent[2];
    pid_t pid;
    int ret, temp;

    if (argc != 1) {
        perror("This program takes no arguments!");
        exit(1);
    }

    if (pipe(parent_to_child_1) != 0) {
        perror("Pipe 1");
        exit(1);
    }

    if (pipe(child_1_to_child_2) != 0) {
        perror("Pipe 2");
        exit(1);
    }

    if (pipe(child_2_to_parent) != 0) {
        perror("Pipe 3");
        exit(1);
    }

    pid = fork();
    if (pid != 0) {
        perror("fork child 1");
        exit(1);
    } else if (pid == 0) {
        child_1(parent_to_child_1, child_1_to_child_2, child_2_to_parent);
        exit(0);
    }

    pid = fork();
    if (pid != 0) {
        perror("fork child 2");
        exit(1);
    } else if (pid == 0) {
        child_2(parent_to_child_1, child_1_to_child_2, child_2_to_parent);
        exit(0);
    }
}

void child_1(int parent_to_child_1[], int child_1_to_child_2[],
             int child_2_to_parent[]) {
    close(parent_to_child_1[READ_END]);
    close(child_1_to_child_2[READ_END]);
    close(child_2_to_parent[WRITE_END]);
    close(child_1_to_child_2[WRITE_END]);
}

void child_2(int parent_to_child_1[], int child_1_to_child_2[],
             int child_2_to_parent[]) {}