#include "mexec.h"
#include "parse.h"
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char** argv) {
    int size = 8, n_cmds = 0;
    char*** cmds = parse_commands(argv, argc, &size, &n_cmds);
    if (n_cmds <= 0 || cmds == NULL) {
        fprintf(stderr, "No commands parsed\n");
        cleanup_and_exit(cmds, n_cmds, EXIT_FAILURE);
    }

    int n_pipes = n_cmds > 1 ? n_cmds - 1 : 0;
    int pipes[n_pipes][2];
    if (n_pipes > 0 && n_pipes != 0) {
        cleanup_and_exit(cmds, n_cmds, EXIT_FAILURE);
    }

    pid_t pids[n_cmds];
    if (fork_children(cmds, n_cmds, pipes, n_pipes, pids) != 0) {
        cleanup_and_exit(cmds, n_cmds, EXIT_FAILURE);
    }

    int fail = 0;
    if (wait_for_children(pids, n_cmds, &fail) != 0) {
        perror("Waitpid");
        cleanup_and_exit(cmds, n_cmds, EXIT_FAILURE);
    }

    free_cmds(cmds, n_cmds);

    return fail ? EXIT_FAILURE : EXIT_SUCCESS;
}

/* --- EXTERNAL FUNCTIONS --- */

void close_pipes(int pipes[][2], int n_pipes) {
    for (int i = 0; i < n_pipes; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
}

int setup_pipes(int pipes[][2], int n_pipes) {
    for (int i = 0; i < n_pipes; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            close_pipes(pipes, i);
            return 1;
        }
    }
    return 0;
}

int fork_children(char*** cmds, int n_cmds, int pipes[][2], int n_pipes, pid_t pids[]) {
    for (int i = 0; i < n_cmds; i++) {
        pids[i] = fork();
        if (pids[i] == -1) {
            perror("fork");
            close_pipes(pipes, n_pipes);
            return 1;
        }

        if (pids[i] == 0) {
            if (i > 0) {
                dup2(pipes[i - 1][0], STDIN_FILENO);
            }
            if (i < n_pipes) {
                dup2(pipes[i][1], STDOUT_FILENO);
            }
            close_pipes(pipes, n_pipes);
            execvp(cmds[i][0], cmds[i]);
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    }
    close_pipes(pipes, n_pipes);
    return 0;
}

int wait_for_children(pid_t pids[], int n_cmds, int* fail) {
    int status = 0;
    for (int i = 0; i < n_cmds; i++) {
        if (waitpid(pids[i], &status, 0) == -1) {
            perror("waitpid");
            return 1;
        }
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            *fail = 1;
            return 1;
        }
    }
    return 0;
}

void cleanup_and_exit(char*** cmds, int n_cmds, int code) {
    free_cmds(cmds, n_cmds);
    exit(code);
}
