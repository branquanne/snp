#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
	int fd;

	if (argc > 1) {
		fprintf(stderr, "This program takes no arguments\n");
		return 1;
	}
	if ((fd = open("file", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR)) <
		0) {
		perror(argv[0]);
		return 1;
	}

	printf("Hej nummer 1\n");
	if (dup2(fd, STDOUT_FILENO) < 0) {
		perror(argv[0]);
		close(fd);
		return 1;
	}
	printf("Hej nummer 2\n");
	close(fd);

	return 0;
}
