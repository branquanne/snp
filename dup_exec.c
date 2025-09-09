#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
	int fd;

	if (argc > 1) {
		fprintf(stderr, "This program takes no argument\n");
		return 1;
	}
	if ((fd = open("file", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR)) <
		0) {
		perror(argv[0]);
		return 1;
	}

	if (dup2(fd, STDOUT_FILENO) < 0) {
		perror(argv[0]);
		close(fd);
		return 1;
	}
	execlp("./myprog", "myprog", (char *)NULL);
	perror("exec failed\n");

	return 1;
}
