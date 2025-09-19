#include "parse.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/**
 * @file mexec.c
 * @brief Executes a sequence of commands, connecting them with pipes if needed.
 *
 * This program reads commands from the command line or a file, sets up the necessary pipes,
 * forks child processes to run each command, connects their input and output as needed,
 * waits for all child processes to finish, and cleans up resources before exiting.
 *
 * Usage:
 *   ./mexec [filename]
 * Or:
 *   ./mexec
 *
 * If a filename is given, commands are read from the file; otherwise, from standard input.
 *
 * Each command is executed in its own process. If multiple commands are given,
 * they are connected in a pipeline (the output of one is the input of the next).
 */

int setup_pipes(int pipes[][2], int n_pipes);
void close_pipes(int pipes[][2], int n_pipes);
int fork_children(char*** cmds, int n_cmds, int pipes[][2], int n_pipes, pid_t pids[]);
int wait_for_children(pid_t pids[], int n_cmds, int* fail);
void cleanup_and_exit(char*** cmds, int n_cmds, int code);

/**
 * Main function.
 * Reads commands, sets up pipes, runs each command in a child process,
 * waits for all children to finish and then cleans up.
 *
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line argument strings.
 * @return EXIT_SUCCESS if all commands worked, EXIT_FAILURE otherwise.
 */
int main(int argc, char** argv) {
    int size = 8, n_cmds = 0;
    char*** cmds = parse_cmds(argv, argc, &size, &n_cmds);
    if (n_cmds <= 0 || cmds == NULL) {
        fprintf(stderr, "No commands parsed\n");
        cleanup_and_exit(cmds, n_cmds, EXIT_FAILURE);
    }

    int n_pipes = n_cmds > 1 ? n_cmds - 1 : 0; // If there are no commands, no pipes are needed
    int pipes[n_pipes][2];
    if (n_pipes > 0 && setup_pipes(pipes, n_pipes) != 0) {
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

/* --- HELPER FUNCTIONS --- */

/**
 * Create pipes for communication between commands.
 * Returns 0 if all pipes are created, 1 if any pipe fails.
 *
 * @param pipes Array to store pipe file descriptors.
 * @param n_pipes Number of pipes to create.
 * @return 0 on success, 1 on error.
 */
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

/**
 * Close all pipes.
 *
 * @param pipes Array of pipes.
 * @param n_pipes Number of pipes.
 */
void close_pipes(int pipes[][2], int n_pipes) {
    for (int i = 0; i < n_pipes; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
}

/**
 * Forks a child process for each command.
 * Sets up input/output pipes for each child.
 * Returns 0 if all children are forked, 1 if any fork fails.
 *
 * @param cmds Array of commands.
 * @param n_cmds Number of commands.
 * @param pipes Array of pipes.
 * @param n_pipes Number of pipes.
 * @param pids Array to store child process IDs.
 * @return 0 on success, 1 on error.
 */
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

/**
 * Waits for all child processes to finish.
 * Sets *fail to 1 if any child fails.
 *
 * @param pids Array of child process IDs.
 * @param n_cmds Number of children.
 * @param fail Pointer to int, set to 1 if any child fails.
 * @return 0 if all succeed, 1 if any fail.
 */
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

/**
 * Frees memory for commands and exits with the given code.
 *
 * @param cmds Array of commands.
 * @param n_cmds Number of commands.
 * @param code Exit code.
 */
void cleanup_and_exit(char*** cmds, int n_cmds, int code) {
    free_cmds(cmds, n_cmds);
    exit(code);
}
