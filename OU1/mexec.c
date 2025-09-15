#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_LINE_SIZE 1024

static char** parseLine(char* buf) {
    int size = 8;
    char** args = malloc(size * sizeof(char*));
    if (!args) {
        perror("args malloc");
        exit(EXIT_FAILURE);
    }
    char* token =
        strtok(buf, " \t\n");  // Use the separator "new line" as specified

    int i = 0;
    while (token != NULL) {
        if (i >= size - 1) {
            size *= 2;
            args = realloc(args, size * sizeof(char*));
            if (!args) {
                perror("args realloc");
                exit(EXIT_FAILURE);
            }
        }

        args[i++] = strdup(token);
        if (!args[i - 1]) {
            perror("args strdup");
            exit(EXIT_FAILURE);
        }
        token = strtok(NULL, " \t\n");
    }
    args[i] = NULL;
    return args;
}

int main(int argc, char** argv) {
    FILE* in = stdin;
    char line[MAX_LINE_SIZE];

    int size = 8;
    char*** cmds = malloc(size * sizeof(char**));
    if (!cmds) {
        perror("cmds malloc");
        exit(EXIT_FAILURE);
    }
    int n_cmds = 0;

    if (argc > 2) {
        fprintf(stderr, "Usage: %s [file]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    if (argc == 2) {
        in = fopen(argv[1], "r");
        if (!in) {
            perror("in fopen");
            exit(EXIT_FAILURE);
        }
    }

    while (fgets(line, sizeof(line), in)) {
        char* temp = line;
        while (*temp == ' ' || *temp == '\t' || *temp == '\n') {
            temp++;
        }

        if (*temp == '\0') {
            continue;
        }

        if (n_cmds >= size - 1) {
            size *= 2;
            cmds = realloc(cmds, size * sizeof(char**));
            if (!cmds) {
                perror("cmds realloc");
                exit(EXIT_FAILURE);
            }
        }
        cmds[n_cmds++] = parseLine(line);
    }
    if (in != stdin) {
        fclose(in);
    }

    int pipes[size - 1][2];
    for (int i = 0; i < n_cmds - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    pid_t pids[size];
    for (int i = 0; i < n_cmds; i++) {
        pids[i] = fork();
        if (pids[i] == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
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

    int status, fail = 0;
    for (int i = 0; i < n_cmds; i++) {
        if (waitpid(pids[i], &status, 0) == -1) {
            perror("waitpid");
            exit(EXIT_FAILURE);
        }
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            fail = 1;
        }
    }

    for (int i = 0; i < n_cmds; i++) {
        for (int j = 0; cmds[i][j] != NULL; j++) {
            free(cmds[i][j]);
        }
        free(cmds[i]);
    }
    free(cmds);

    return fail ? EXIT_FAILURE : EXIT_SUCCESS;
}
