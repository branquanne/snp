#include "parse.h"
#include <sys/wait.h>

void close_pipes(int pipes[][2], int n_pipes);
int setup_pipes(int pipes[][2], int n_pipes);
int wait_status(pid_t pids[], int n_cmds, int* fail);
int fork_children(char*** cmds, int n_cmds, int pipes[][2], pid_t pids[]);

int main(int argc, char** argv) {
    int size = 8, n_cmds = 0;
    char*** cmds = parse_args(argv, argc, &size, &n_cmds);
    if (n_cmds <= 0 || cmds == NULL) {
        fprintf(stderr, "No commands parsed\n");
        exit(EXIT_FAILURE);
    }

    int pipes[n_cmds - 1][2];
    if (setup_pipes(pipes, n_cmds) != 0) {
        free_cmds(cmds, n_cmds);
        exit(EXIT_FAILURE);
    }

    pid_t pids[n_cmds];
    if (fork_children(cmds, n_cmds, pipes, pids) != 0) {
        free_cmds(cmds, n_cmds);
        exit(EXIT_FAILURE);
    }

    int fail = 0;
    if (wait_status(pids, n_cmds, &fail) != 0) {
        perror("Waitpid");
        free_cmds(cmds, n_cmds);
        exit(EXIT_FAILURE);
    }

    free_cmds(cmds, n_cmds);

    return fail ? EXIT_FAILURE : EXIT_SUCCESS;
}

void close_pipes(int pipes[][2], int n_pipes) {
    for (int i = 0; i < n_pipes; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
}

int setup_pipes(int pipes[][2], int n_pipes) {
    for (int i = 0; i < n_pipes - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            close_pipes(pipes, i);
            return 1;
        }
    }
    return 0;
}

int fork_children(char*** cmds, int n_cmds, int pipes[][2], pid_t pids[]) {
    for (int i = 0; i < n_cmds; i++) {
        pids[i] = fork();
        if (pids[i] == -1) {
            perror("fork");
            close_pipes(pipes, n_cmds - 1);

            return 1;
        }

        if (pids[i] == 0) {
            if (i > 0) {
                dup2(pipes[i - 1][0], STDIN_FILENO);
            }
            if (i < n_cmds - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
            }

            for (int j = 0; j < n_cmds - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            execvp(cmds[i][0], cmds[i]);
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < n_cmds - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
    return 0;
}

int wait_status(pid_t pids[], int n_cmds, int* fail) {

    int status = 0;
    for (int i = 0; i < n_cmds; i++) {
        if (waitpid(pids[i], &status, 0) == -1) {
            perror("waitpid");
            return 1;
        }
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            *fail = 1;
        }
    }
    return 0;
}
