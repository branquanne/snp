#include <stdio.h>
#include <stdlib.h>

#define STRING_SIZE 100

char* read_string(FILE* stream);

int main(int argc, char** argv)
{
    FILE* stream = fopen("textfil.txt", "r");
    if (!stream) {
        perror("fopen failed!");
        return 0;
    }

    char* endString = read_string(stream);

    printf("The final string from the file is: %s\n", endString);

    fclose(stream);
    free(endString);

    return 0;
}

char* read_string(FILE* stream)
{
    char* string = malloc(STRING_SIZE);
    if (string == NULL) {
        perror("Memory allocation failed!");
    }
    int c;
    int i = 0;
    while ((c = fgetc(stream)) != EOF && i < (STRING_SIZE - 1)) {
        string[i++] = (char)c;
    }

    string[i] = '\0';

    return string;
}

void write_string(FILE* stream)
{
}
