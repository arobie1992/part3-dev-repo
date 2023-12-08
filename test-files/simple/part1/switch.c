#include <stdio.h>

void foo(int a) {
    switch(a) {
    case 1:
        printf("is 1\n");
        break;
    case 2:
        printf("is 2\n");
    case 3:
        printf("might be 2 or 3\n");
        break;
    }
    printf("finished\n");
}

int main() {
    foo(2);
}