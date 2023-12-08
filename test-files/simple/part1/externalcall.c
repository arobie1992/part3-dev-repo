#include <stdio.h>
#include "externalfunc.h"

void foo(int a) {
    if(a == 1) {
        printf("cond\n");
    }
    printf("after\n");
}

int main() {
    int a = 1;
    foo(a);
    ext(a);
}