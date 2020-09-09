#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main() {
    int fds[2];
    if (pipe(fds) < 0) {
        printf("Create pipe failed!\n");
        return -1;
    }

    // Set non-block
    if (fcntl(fds[1], F_SETFL, O_NONBLOCK) < 0) {
        printf("Set non-block failed!\n");
        return -1;
    }

    int n = 0;
    while (1) {
        if (write(fds[1], "z", 1) != 1) {
            break;
        }
        n++;
    }

    printf("pipe buffer len:%d PIPE_BUF:%ld\n", n, fpathconf(fds[1], _PC_PIPE_BUF));
    return 0;
}