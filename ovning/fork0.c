#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
    printf("Utskrift 1\n");
    fork();
    fork();
    printf("Utskrift 2\n");

    exit(0);
}