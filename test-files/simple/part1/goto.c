#include <stdio.h>

void foo(int a) {
    goto a;
    b: printf("before\n");
    goto c;
    a: printf("after\n");
    goto b;
    c:;
}

int main() {
    foo(1);
}