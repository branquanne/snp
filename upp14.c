#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STRING_SIZE 100

typedef struct {
    char arr[STRING_SIZE];
    int len;
} string;

void string_init(string* string, const char* src);

int main(int argc, char** argv) {

    string* s = malloc(sizeof(string));

    char* src = "Hello";
    string_init(s, src);

    printf("The string is: %s\n", s->arr);

    free(s);
    return 0;
}

void string_init(string* string, const char* src) {
    strcpy(string->arr, src);
    string->len = strlen(string->arr);
}