#include "tree_helper.c"
#include <stdio.h>
#include <stdlib.h>

void print_connections(Connection* connections, int n, char start);

int main(int argc, char** argv) {
    int n;
    Connection* connections = read_connections_file(argv[1], &n);
    print_connections(connections, n, connections[0].from);

    return 0;
}

void print_connections(Connection* connections, int n, char start) {
    int found = 0;
    for (int i = 0; i < n; i++) {
        if (connections[i].from == start) {
            printf("%c", connections[i].from);
            found = 1;
            print_connections(connections, n, connections[i].to);
        }
    }
    if (!found) {
        printf("%c\n", start);
    }
}