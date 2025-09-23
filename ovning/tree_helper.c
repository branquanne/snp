/**
 * @brief Contains code to help solve recursion exercises
 *        related to a tree of connected letters.
 */
#include <stdio.h>
#include <stdlib.h>

#define BUF_SIZE 256

/**
 * @brief Represents a letter connection in a tree, ex 'A->B'.
 */
typedef struct Connection {
	char from;
	char to;
} Connection;

/**
 * @brief Read a list of letter connections from file.
 *
 * @param filename   File to read from.
 * @param count      Returns the amount of connections.
 * @return           Array of all connections or NULL if memory allocation
 *                   failed.
 */
Connection *read_connections_file(char *filename, int *count) {
	FILE *f;

	if ((f = fopen(filename, "r")) == NULL) {
		perror("file open");
		return NULL;
	};

	char buffer[BUF_SIZE];
	int connections_max_length = 1;
	Connection *connections = calloc(1, sizeof(Connection));

	int i = 0;
	// Loop over every line
	while (fgets(buffer, BUF_SIZE, f)) {
		// Every line is in format 'A->B'
		connections[i].from = buffer[0];
		connections[i].to = buffer[3];
		i++;
		// Reallocate connections array if the allocated memory is exceeded
		if (i >= connections_max_length) {
			connections_max_length = connections_max_length * 2;
			connections = realloc(connections,
								  sizeof(Connection) * connections_max_length);
		}
	}

	*count = i;

	fclose(f);

	return connections;
}
