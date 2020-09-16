#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int my_dup2(int oldfd, int newfd) {
    if (newfd == oldfd) {
        return newfd;
    }

    const int max_fd_count = 255;
    int* fds = (int*)(malloc(max_fd_count * sizeof(int)));

    int count = 0;
    int current_fd = -1;
    while (count < max_fd_count) {
        current_fd = dup(oldfd);
        if (current_fd == -1) {
            break;
        } else if (current_fd == newfd) {
            break;
        } else {
            fds[count] = current_fd;
            ++count;
        }
    }

    for (int i = 0; i < count; ++i) {
        close(fds[i]);
    }
    free(fds);

    if (current_fd != newfd) {
        return -1;
    }
    return newfd;
}

#define STDOUT 1

int main() {
    int fd = my_dup2(STDOUT, 8);
    if (fd == -1) {
        printf("Dup2 failed, errno:%d\n", errno);
        return -1;
    }
    printf("Dup2 success, fd:%d\n", fd);
    write(fd, "hello world!\n", 14);
    return 0;
}