#ifndef PARSE_H
#define PARSE_H

#define MAX_LINE_SIZE 1024

/**
 * @brief Splits a line into arguments using whitespace as separators.
 *
 * Allocates and returns an array of strings, one for each argument.
 * The last element is set to NULL.
 *
 * @param buf The input line to parse.
 * @return Array of argument strings (NULL-terminated).
 */
char** parse_line(char* buf);

/**
 * @brief Reads and parses commands from stdin or a file.
 *
 * Allocates and returns an array of command argument arrays.
 * Each command is split into arguments using parse_line.
 *
 * @param buf Command-line arguments (argv).
 * @param argc Number of command-line arguments.
 * @param size Pointer to the initial size of the command array (may be updated).
 * @param n_cmds Pointer to the number of commands found (updated).
 * @return Array of command argument arrays.
 */
char*** parse_cmds(char** buf, int argc, int* size, int* n_cmds);

/**
 * @brief Frees memory for an array of argument strings.
 *
 * @param args Array of argument strings.
 * @param n_args Number of arguments.
 */
void free_line(char** args, int n_args);

/**
 * @brief Frees memory for an array of command argument arrays.
 *
 * @param cmds Array of command argument arrays.
 * @param n_cmds Number of commands.
 */
void free_cmds(char*** cmds, int n_cmds);

#endif
