#ifndef MEXEC_H
#define MEXEC_H

#include <stdlib.h>
#include <sys/types.h>

void close_pipes(int pipes[][2], int n_pipes);
int setup_pipes(int pipes[][2], int n_pipes);
int wait_for_children(pid_t pids[], int n_cmds, int* fail);
int fork_children(char*** cmds, int n_cmds, int pipes[][2], pid_t pids[]);

#endif
