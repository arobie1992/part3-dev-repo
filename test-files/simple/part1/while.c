#include <stdio.h>

void foo(int a) {
    int i = 0;
    while(i < a) {
        printf("hi%d\n", i);
        i++;
    }
    printf("bye\n");
}

int main() {
    foo(1);
}