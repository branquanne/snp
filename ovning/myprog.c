#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
	if (argc != 1) {
		fprintf(stderr, "%s: Too many arguments (0 expected)\n", argv[0]);
		return 1;
	}

	printf("Utskrift nummer 1\n");

	fprintf(stdout, "Utskrift nummer 2\n");
	fprintf(stderr, "Utskrift nummer 2.5\n");

	write(STDOUT_FILENO, "Utskrift nummer 3\n", 18);

	return 0;
}
