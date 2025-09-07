#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STRING_SIZE 100

typedef struct {
    char arr[STRING_SIZE];
    int len;
} string;

void string_init(string *string, const char *src);
void print_string(string *string);
char *read_string(FILE *file);

int main(int argc, char **argv) {
    FILE *file = fopen("textfil.txt", "r");
    if (!file) {
        perror("Failed to open file");
    }

    string *s = malloc(sizeof(string));
    if (s == NULL) {
        perror("Failed to allocate memory for struct");
    }

    char *src = read_string(file);

    string_init(s, src);

    print_string(s);

    free(src);
    free(s);
    return 0;
}

void string_init(string *string, const char *src) {
    strcpy(string->arr, src);
    string->len = strlen(string->arr);
}

void print_string(string *string) {
    printf("The structs string is: %s\n", string->arr);
}

char *read_string(FILE *file) {
    char *src = malloc(STRING_SIZE);
    if (src == NULL) {
        perror("Failed to allocate memory for src string");
    }

    int i = 0;
    int c;

    while ((c = fgetc(file)) != EOF && i < (STRING_SIZE - 1)) {
        src[i++] = (char)c;
    }

    src[i] = '\0';

    return src;
}
