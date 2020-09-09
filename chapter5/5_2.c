#include <stdio.h>

#define MAX_LINE 4

int main() {
    char buffer[MAX_LINE];

    while (fgets(buffer, MAX_LINE, stdin) != NULL) {
        if (fputs(buffer, stdout) == EOF) {
            printf("fputs error!\n");
            return -1;
        }
    }

    if (ferror(stdin) != 0) {
        printf("Read stdin error!\n");
        return -1;
    }

    return 0;
}