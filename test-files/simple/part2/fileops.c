#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *fileName = "fileops_test_file.txt";

void writeFile() {
    FILE *f = fopen(fileName, "w");
    printf("Writing to file\n");
    fwrite("test\n", 5, 1, f);
    fclose(f);
}

void readFile() {
    FILE *f = fopen(fileName, "r");
    char *buff = malloc(sizeof(char)*6);
    fread(buff, 6, 1, f);
    if(strcmp(buff, "test\n")) {
        printf("Someone changed the contents of the file :O\n");
    }
    fclose(f);
    free(buff);
}

void getcFile() {
    FILE *f = fopen(fileName, "r");
    char c = getc(f);
    switch(c) {
        case 't':
            printf("Everthing is good :)\n");
            break;
        default:
            printf("Someone was messing with the code >:(\n");
            break;
    }
    fclose(f);
}

int main() {
    writeFile();
    readFile();
    getcFile();
    return 0;
}