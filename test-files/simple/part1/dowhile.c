#include <stdio.h>

void foo(int a) {
    int i = 0;
    do {
        printf("hi%d\n", i);
        i++;
    } while(i < a);
    printf("bye\n");
}

int main() {
    foo(1);
}