#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

char** parse_line(char* buffer) {
    int size = 8;
    char** args = malloc(size * sizeof(char*));
    char* token = strtok(buffer, " \t\n");

    int i = 0;
    while (token != NULL) {
        if (i >= size - 1) {
            size *= 2;
            args = realloc(args, size * sizeof(char*));
        }

        args[i++] = strdup(token);
        token = strtok(NULL, " \t\n");
    }
    args[i] = NULL;
    return args;
}

void execute_command(char** args) { execvp(args[0], args); }

int main(int argc, char* argv[]) {
    char line[256];
    printf("Write a command: ");
    if (fgets(line, sizeof(line), stdin) == NULL) {
        perror("fgets");
        exit(1);
    }

    char** args = parse_line(line);
    for (int i = 0; args[i] != NULL; i++) {
        printf("args[%d]: %s\n", i, args[i]);
    }

    execute_command(args);

    for (int i = 0; args[i] != NULL; i++) {
        free(args[i]);
    }
    free(args);

    exit(0);
}
