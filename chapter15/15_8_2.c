#include <stdio.h>

int main() {
    printf("stdout\n");
    perror("stderr");
    return 0;
}