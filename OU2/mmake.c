#include "parser.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

/**
 * @file mmake.c
 * @brief A simplified version of the 'make' tool.
 *
 * This program reads build rules from a mmakefile, determines which targets need to be rebuilt
 * based on their prerequisites and modification times, and executes the specified commands
 * to build those targets. It supports the usage of different flags, enabling the user to indicate
 * a forced rebuild, a silenced build and the option to use a different mmakefile.
 *
 * Usage:
 *   ./mmake [-f MAKEFILE] [-B] [-s] [TARGET ...]
 *
 * If no target is specified, the default target from the mmakefile is built.
 * Each target is built recursively, checking if the prerequisites are up-to-date before building.
 */

void build_target(makefile* mf, const char* target, bool force_rebuild, bool silent);
bool is_outdated(const char* target, const char** prereqs);
void run_command(char** cmds, makefile* mf);
static void usage(void);
bool target_exists(const char* target);
bool prereqs_exist(const char** prereqs);
bool are_prereqs_newer(const char* target, const char** prereqs);
void print_commands(char** cmds);

/**
 * @brief Main function for the mmake program.
 *
 * Parses command-line arguments, opens and parses the makefile, and initiates the build process
 * for the specified targets, or lack there of. Handles options for forcing
 * rebuilds (-B), silencing output (-s), and specifying a different mmakefile (-f).
 *
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line argument strings.
 * @return EXIT_SUCCESS if all targets are built successfully, EXIT_FAILURE otherwise.
 */

int main(int argc, char** argv) {
    bool force_rebuild = false;
    bool silent = false;
    char* mmakefile_name = "mmakefile";

    int flag;
    while ((flag = getopt(argc, argv, "f:Bs")) != -1) {
        switch (flag) {
        case 'B':
            force_rebuild = true;
            break;

        case 'f':
            mmakefile_name = optarg;
            break;

        case 's':
            silent = true;
            break;

        default:
            usage();
            break;
        }
    }

    FILE* fp = fopen(mmakefile_name, "r");
    if (!fp) {
        perror(mmakefile_name);
        exit(EXIT_FAILURE);
    }

    makefile* mf = parse_makefile(fp);
    if (!mf) {
        perror(mmakefile_name);
        fclose(fp);
        exit(EXIT_FAILURE);
    }

    fclose(fp);

    int n_targets = argc - optind;
    const char* target;
    if (n_targets > 0) {
        for (int i = optind; i < argc; i++) {
            target = argv[i];
            build_target(mf, target, force_rebuild, silent);
        }
    } else {
        target = makefile_default_target(mf);
        build_target(mf, target, force_rebuild, silent);
    }

    makefile_del(mf);
    exit(EXIT_SUCCESS);
}

/**
 * @brief Recursively builds the specified target.
 *
 * If the target has a rule, builds all its prerequisites first. Determines if the target needs to be
 * rebuilt (based on timestamps or force flag). If rebuild is needed, prints the command (unless silent)
 * and executes it. If the target does not have a rule but exists as a file, does nothing. If the target
 * does not exist and has no rule, prints an error and exits.
 *
 * @param mf Pointer to the parsed makefile structure.
 * @param target Name of the target to build.
 * @param force_rebuild If true, always rebuild the target.
 * @param silent If true, suppress command output.
 */

void build_target(makefile* mf, const char* target, bool force_rebuild, bool silent) {
    rule* rule = makefile_rule(mf, target);
    if (!rule) {
        struct stat source_stat;
        if (stat(target, &source_stat) != 0) {
            fprintf(stderr, "Could not extract rules\n");
            makefile_del(mf);
            exit(EXIT_FAILURE);
        }
        return;
    }

    const char** prereqs = rule_prereq(rule);
    for (int i = 0; prereqs[i]; i++) {
        build_target(mf, prereqs[i], force_rebuild, silent);
    }

    bool needs_rebuild = force_rebuild || is_outdated(target, prereqs);
    if (needs_rebuild) {
        char** cmds = rule_cmd(rule);
        if (!silent) {
            print_commands(cmds);
        }
        run_command(cmds, mf);
    }
}

/**
 * @brief Determines if the target file is outdated.
 *
 * Returns true if the target file does not exist, any prerequisite does not exist,
 * or any prerequisite is newer than the target. Otherwise, returns false.
 *
 * @param target Name of the target file.
 * @param prereqs Array of prerequisite file names.
 * @return true if the target is outdated, false otherwise.
 */

bool is_outdated(const char* target, const char** prereqs) {
    return target_exists(target) || prereqs_exist(prereqs) || are_prereqs_newer(target, prereqs) ? true : false;
}

/**
 * @brief Executes the given command using fork and execvp.
 *
 * Forks a child process to run the command. If execvp fails, prints an error and exits.
 * Waits for the child process to finish. If the command fails, cleans up and exits with error.
 *
 * @param cmds Array of command arguments.
 * @param mf Pointer to the parsed makefile structure (for cleanup).
 */

void run_command(char** cmds, makefile* mf) {
    pid_t pid = fork();

    if (pid == -1) {
        fprintf(stderr, "Unable to fork processID: %d\n", pid);
        makefile_del(mf);
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        execvp(cmds[0], (char* const*)cmds);
        perror(cmds[0]);
        makefile_del(mf);
        exit(EXIT_FAILURE);
    } else {
        int status;
        if (wait(&status) == -1) {
            fprintf(stderr, "Failed to wait for child process\n");
            makefile_del(mf);
            exit(EXIT_FAILURE);
        }
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            makefile_del(mf);
            exit(EXIT_FAILURE);
        }
    }
}

void print_commands(char** cmds) {
    for (int i = 0; cmds[i]; i++) {
        printf("%s", cmds[i]);
        if (cmds[i + 1]) {
            printf(" ");
        }
    }
    printf("\n");
}

/**
 * @brief Prints usage information for the program and exits.
 *
 * Called when invalid command-line arguments are provided.
 */

static void usage(void) {
    fprintf(stderr, "mmake [-f MAKEFILE] [-B] [-s] [TARGET ...]\n");
    exit(EXIT_FAILURE);
}

bool target_exists(const char* target) {
    struct stat target_stat;
    if (stat(target, &target_stat) != 0) {
        return true;
    }
    return false;
}

bool prereqs_exist(const char** prereqs) {
    for (int i = 0; prereqs[i]; i++) {
        struct stat prereqs_stat;
        if (stat(prereqs[i], &prereqs_stat) != 0) {
            return true;
        }
    }
    return false;
}

bool are_prereqs_newer(const char* target, const char** prereqs) {
    struct stat target_stat, prereqs_stat;
    for (int i = 0; prereqs[i]; i++) {
        if (prereqs_stat.st_mtime > target_stat.st_mtime) {
            return true;
        }
    }
    return false;
}
