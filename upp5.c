#include <stdio.h>

#define STRING_SIZE 100

int main(void)
{
    char arr[STRING_SIZE];
    int i = 0;
    int c;

    while ((c = fgetc(stdin)) != '\n' /*&& c != EOF && i != STRING_SIZE - 1*/) {
        arr[i++] = (char)c;
    }

    arr[i] = '\0';

    printf("String: %s\n", arr);

    return 0;
}
