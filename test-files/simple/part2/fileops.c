#include <stdio.h>

const char *fileName = "fileops_test_file.txt";

void writeFile() {
    FILE *f = fopen(fileName, "w");
    printf("Writing to file\n");
    fwrite("test\n", 5, 1, f);
    fclose(f);
}

void readFile() {
    FILE *f = fopen(fileName, "r");
    char buff[6];
    fread(buff, 6, 1, f);
    printf("Read from file: %s\n", buff);
    fclose(f);
}

void getcFile() {
    FILE *f = fopen(fileName, "r");
    char c = getc(f);
    printf("First character is: %c\n", c);
    fclose(f);
}

int main() {
    writeFile();
    readFile();
    getcFile();
    return 0;
}