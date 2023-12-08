#include <stdio.h>

void foo(int a) {
    for(int i = 0; i < a; i++) {
        printf("hi%d\n", i);
    }
    printf("bye\n");
}

int main() {
    foo(1);
}