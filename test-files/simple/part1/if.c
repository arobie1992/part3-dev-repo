#include <stdio.h>

void foo(int a) {
    if(a == 1) {
        printf("cond\n");
    }
    printf("after\n");
}

int main() {
    foo(1);
}