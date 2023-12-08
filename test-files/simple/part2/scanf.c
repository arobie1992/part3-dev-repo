#include <stdio.h>

int main() {
    int num_loops = -1;
    scanf("%d", &num_loops);
    for(int i = 0; i < num_loops; i++) {
        printf("on iteration %d\n", i);
    }
}