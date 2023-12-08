#include <stdio.h>

int main() {
    int user_input = -1;
    scanf("%d", &user_input);
    switch(user_input) {
        case 0:
            printf("You picked zero\n");
            break;
        case 1:
            printf("You picked one\n");
            break;
        default:
            printf("You picked some other value\n");
            break;
    }
    return 0;
}