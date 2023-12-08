#include <stdio.h>

void foo(int a) {
    if(a == 1) {
        printf("cond1\n");
    } else if(a == 2) {
        printf("cond2\n");
    } else {
        printf("alt\n");
    }
    printf("after\n");
}

int main() {
    foo(5);
}