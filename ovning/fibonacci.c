#include <stdio.h>
#include <stdlib.h>

void fib(int n, int prev1, int prev2) {
    if (n < 1) {
        return;
    }
    int curr = prev1 + prev2;
    printf("%d ", curr);
    return fib(n - 1, curr, prev1);
}

int main(void) {

    int n;
    if (scanf("%d", &n) != 1) {
        printf("Invalid\n");
    }
    if (n < 1) {
        printf("Too low a number");
    }
    printf("0 1 ");
    if (n > 2) {
        fib(n - 2, 1, 0);
    }
    printf("\n");
    return 0;
}