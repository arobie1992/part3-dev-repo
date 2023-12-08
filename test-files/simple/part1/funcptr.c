#include <stdio.h>

void foo(int a) {
    printf("hi%d\n", a);
}

int main() {
    void (*fp)(int) = &foo;
    (*fp)(1);
}