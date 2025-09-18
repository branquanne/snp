#ifndef PARSE_H
#define PARSE_H

#define MAX_LINE_SIZE 1024

char** parse_line(char* buf);
char*** parse_cmds(char** buf, int argc, int* size, int* n_cmds);
void free_cmds(char*** cmds, int n_cmds);

#endif
