#include <stdio.h>
#include <unistd.h>

int main() {
    if (unlink("/dev/fd/1") != 0) {
        printf("unlink failed\n");
        return -1;
    }
    printf("unlink success.\n");
    return 0;
}