#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#define OPEN_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)

bool redirect(int fd) {
    if (dup2(fd, 0) < 0) { return false; }
    if (dup2(fd, 1) < 0) { return false; }
    if (dup2(fd, 2) < 0) { return false; }
    if (fd > 2) { close(fd); }
    return true;
}

int main() {
    const char* file = "test.txt";
    int fd = open(file, O_CREAT | O_TRUNC | O_RDWR, OPEN_MODE);
    if (fd == -1) {
        printf("Open file[%s] failed!", file);
        return -1;
    }

    // If succeed, stdin/stdout/stderr point to file.
    if (!redirect(fd)) {
        printf("redirect failed!\n");
        return -1;
    }

    printf("Hello World\n");
    printf("What are you doing\n");
    fflush(stdout);
    perror("error\n");
    perror("error2\n");

    return 0;
}