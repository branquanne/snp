#include "parser.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

void build_target(makefile* mf, const char* target, bool force_rebuild, bool silent);
bool target_is_outdated(const char* target, const char** prereqs);
void run_command(char** cmds, makefile* mf);
static void usage(void);

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
        fprintf(stderr, "Could not open %s\n", mmakefile_name);
        exit(EXIT_FAILURE);
    }

    makefile* mf = parse_makefile(fp);
    if (!mf) {
        fprintf(stderr, "Could not parse mmakefile\n");
        fclose(fp);
        makefile_del(mf);
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

    // Traverse rules

    // Check prereqs

    // Run commands to build revursively

    makefile_del(mf);
    return 0;
}

void build_target(makefile* mf, const char* target, bool force_rebuild, bool silent) {
    rule* rule = makefile_rule(mf, target);
    if (!rule) {
        struct stat source_stat;
        if (stat(target, &source_stat) == 0) {
            return;
        }
        fprintf(stderr, "Could not extract rules\n");
        makefile_del(mf);
        exit(EXIT_FAILURE);
    }

    const char** prereqs = rule_prereq(rule);
    for (int i = 0; prereqs[i]; i++) {
        target = prereqs[i];
        build_target(mf, target, force_rebuild, silent);
    }

    bool needs_rebuild = force_rebuild || target_is_outdated(target, prereqs);
    if (needs_rebuild) {
        char** cmds = rule_cmd(rule);
        if (!silent) {
            for (int i = 0; cmds[i]; i++) {
                printf("%s ", cmds[i]);
            }
            printf("\n");
        }
        run_command(cmds, mf);
    }
}

bool target_is_outdated(const char* target, const char** prereqs) {
    struct stat target_stat;
    if (stat(target, &target_stat) != 0) {
        return true;
    }

    for (int i = 0; prereqs[i]; i++) {
        struct stat prereqs_stat;
        if (stat(prereqs[i], &prereqs_stat) != 0) {
            return true;
        }
        if (prereqs_stat.st_mtime > target_stat.st_mtime) {
            return true;
        }
    }

    if (time(NULL) - target_stat.st_mtime > 1) {
        return true;
    }

    return false;
}

void run_command(char** cmds, makefile* mf) {
    pid_t pid = fork();

    if (pid == -1) {
        fprintf(stderr, "Unable to fork processID: %d\n", pid);
        makefile_del(mf);
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        execvp(cmds[0], (char* const*)cmds);
        fprintf(stderr, "Failed to execute: %s\n", cmds[0]);
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
            fprintf(stderr, "Command failed: %s\n", cmds[0]);
            makefile_del(mf);
            exit(EXIT_FAILURE);
        }
    }
}

static void usage(void) {
    fprintf(stderr, "mmake [-f MAKEFILE] [-B] [-s] [TARGET ...]\n");
    exit(EXIT_FAILURE);
}
