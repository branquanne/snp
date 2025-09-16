#ifndef PARSE_H
#define PARSE_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_LINE_SIZE 1024

char** parse_line(char* buf);
char*** parse_args(char** buf, int argc, int* size, int* n_cmds);
void free_cmds(char*** cmds, int n_cmds);

#endif // !PARSE_H
