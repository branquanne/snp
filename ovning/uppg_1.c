#include <stdio.h>

int main(void) {
    char string[] = "hej hej";

    int i = 0;
    while (string[i] != '\0') {
        i++;
    }

    printf("Size of string is: %d\n", i);

    return 0;
}
