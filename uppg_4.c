#include <stdio.h>

#define STRING_SIZE 100

int main(void)
{
    char str1[STRING_SIZE];
    char str2[STRING_SIZE];

    int j = 0;
    int i = 0;

    int c;

    while ((c = fgetc(stdin)) != '\n') {
        str1[i++] = (char)c;
    }

    while ((c = fgetc(stdin)) != '\n') {
        str2[j] = (char)c;
        str1[i++] = str2[j++];
    }

    str1[i] = '\0';

    printf("The conctatinated string is: %s\n", str1);

    return 0;
}
