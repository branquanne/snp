#include <stdio.h>
#include <stdlib.h>

int factorial(int n) {
    if (n == 1) {
        return n;
    } else {
        return n * factorial(n - 1);
    }
}

int main(void) {
    int i = factorial(5);
    printf("%d\n", i);

    return 0;
}