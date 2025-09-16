#include "parse.h"

void free_args(char** args, int n_args);

char** parse_line(char* buf) {
    int size = 8;
    char** args = malloc(size * sizeof(char*));
    if (!args) {
        perror("args malloc");
        exit(EXIT_FAILURE);
    }
    char* token = strtok(buf, " \t\n"); // Use the separator "new line" as specified

    int i = 0;
    while (token != NULL) {
        if (i >= size - 1) {
            size *= 2;
            args = realloc(args, size * sizeof(char*));
            if (!args) {
                perror("args realloc");
                free_args(args, i);
                exit(EXIT_FAILURE);
            }
        }

        args[i++] = strdup(token);
        if (!args[i - 1]) {
            perror("args strdup");
            free_args(args, i);
            exit(EXIT_FAILURE);
        }
        token = strtok(NULL, " \t\n");
    }
    args[i] = NULL;
    return args;
}

char*** parse_args(char** buf, int argc, int* size, int* n_cmds) {
    FILE* in = stdin;
    char line[MAX_LINE_SIZE];

    char*** cmds = malloc(*size * sizeof(char**));
    if (!cmds) {
        perror("cmds malloc");
        exit(EXIT_FAILURE);
    }

    if (argc > 2) {
        fprintf(stderr, "Usage: %s [file]\n", buf[0]);
        free_cmds(cmds, *n_cmds);
        exit(EXIT_FAILURE);
    }
    if (argc == 2) {
        in = fopen(buf[1], "r");
        if (!in) {
            perror("in fopen");
            free_cmds(cmds, *n_cmds);
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

        if (*n_cmds >= *size - 1) {
            *size *= 2;
            cmds = realloc(cmds, *size * sizeof(char**));
            if (!cmds) {
                perror("cmds realloc");
                exit(EXIT_FAILURE);
            }
        }
        cmds[(*n_cmds)++] = parse_line(line);
    }
    if (in != stdin) {
        fclose(in);
    }

    return cmds;
}

void free_args(char** args, int n_args) {
    for (int i = 0; i < n_args; i++) {
        free(args[i]);
    }
    free(args);
}

void free_cmds(char*** cmds, int n_cmds) {
    for (int i = 0; i < n_cmds; i++) {
        for (int j = 0; cmds[i][j] != NULL; j++) {
            free(cmds[i][j]);
        }
        free(cmds[i]);
    }
    free(cmds);
}
