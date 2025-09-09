#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void child_1(int parent_to_child[], int child_to_child[],
			 int child_to_parent[]);
void child_2(int parent_to_child[], int child_to_child[],
			 int child_to_parent[]);

#define READ_END 0
#define WRITE_END 1

int main(int argc, char *argv[]) {
	int parent_to_child[2];
	int child_to_child[2];
	int child_to_parent[2];
	pid_t pid;
	int number, ret;

	/* Test of number of arguments (should be 0) */
	if (argc != 1) {
		fprintf(stderr, "\n%s takes no argument\n\n", argv[0]);
		exit(1);
	}

	/* Open pipes  */
	if (pipe(parent_to_child) != 0) {
		perror("Pipe 1");
		exit(1);
	}
	if (pipe(child_to_child) != 0) {
		perror("Pipe 2");
		exit(1);
	}
	if (pipe(child_to_parent) != 0) {
		perror("Pipe 3");
		exit(1);
	}

	/* Create a new process for child 1 */
	pid = fork();

	if (pid < 0) {
		perror("fork 1\n");
		exit(1);
	} else if (pid == 0) { /* Child 1 */
		child_1(parent_to_child, child_to_child, child_to_parent);
		exit(0);
	}

	/* Parent */
	/* Create a new process for child 2 */
	pid = fork();

	if (pid < 0) {
		perror("fork 2\n");
		exit(1);
	} else if (pid == 0) { /* Child 2 */
		child_2(parent_to_child, child_to_child, child_to_parent);
		exit(0);
	}

	/* Parent */
	int input = 0;

	/* Close non used pipes */
	close(parent_to_child[READ_END]);
	close(child_to_child[READ_END]);
	close(child_to_child[WRITE_END]);
	close(child_to_parent[WRITE_END]);

	/* Do the work */
	do {
		printf("Input an integer: ");
		while ((input = scanf("%d", &number)) != 1) {
			if (input == EOF) {
				break;
			}
			fprintf(
				stderr,
				"Conversion failed - must be an integer or EOF, try again\n");
			getchar();
		}
		if (input != EOF) {
			if (write(parent_to_child[WRITE_END], &number, sizeof(int)) == -1) {
				perror("write to child 1");
				exit(1);
			}
			ret = read(child_to_parent[READ_END], &number, sizeof(int));
			if (ret == -1) {
				perror("read from child 2");
				exit(1);
			}
			printf("The answer is = %d\n\n", number);
		}
	} while (input != EOF);

	/* Work finished close the rest of the pipes */
	close(parent_to_child[WRITE_END]);
	close(child_to_parent[READ_END]);

	/* Wait for both childs */
	for (int i = 0; i < 2; i++)
		if ((pid = wait(NULL)) == -1) {
			perror("Error waiting");
		} else {
			printf("Child %d has terminated\n", pid);
		}

	exit(0);
}

void child_1(int parent_to_child[], int child_to_child[],
			 int child_to_parent[]) {
	int ret, temp_nr;

	/* Close non used pipes */
	close(parent_to_child[WRITE_END]);
	close(child_to_child[READ_END]);
	close(child_to_parent[READ_END]);
	close(child_to_parent[WRITE_END]);

	while ((ret = read(parent_to_child[READ_END], &temp_nr, sizeof(int))) ==
		   sizeof(int)) {
		temp_nr++;
		if (write(child_to_child[WRITE_END], &temp_nr, sizeof(int)) == -1) {
			perror("write to child 2");
			exit(1);
		}
	}

	if (ret == -1) {
		perror("read from parent");
		exit(1);
	}

	/* Work finished close the rest of the pipes */
	close(parent_to_child[READ_END]);
	close(child_to_child[WRITE_END]);

	return;
}

void child_2(int parent_to_child[], int child_to_child[],
			 int child_to_parent[]) {
	int ret, local_nr;

	/* Close non used pipes */
	close(parent_to_child[READ_END]);
	close(parent_to_child[WRITE_END]);
	close(child_to_child[WRITE_END]);
	close(child_to_parent[READ_END]);

	while ((ret = read(child_to_child[READ_END], &local_nr, sizeof(int))) ==
		   sizeof(int)) {
		local_nr *= 10;
		if (write(child_to_parent[WRITE_END], &local_nr, sizeof(int)) == -1) {
			perror("write to parent");
			exit(1);
		}
	}

	if (ret == -1) {
		perror("read from child 1");
		exit(1);
	}

	/* Work finished close the rest of the pipes */
	close(parent_to_child[READ_END]);
	close(child_to_child[WRITE_END]);
}
