#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static const int hole_size = 12345;
static const int write_size = 10;

static const char* hole_file = "./hole_file.dat";

int main() {
    int fd = open(hole_file, O_WRONLY | O_CREAT, 0644);
    if (fd == -1) {
        printf("Open file[%s] failed!\n", hole_file);
        return -1;
    }

    char* buffer = (char*)malloc(write_size);
    for (int i = 0; i < write_size; ++i) {
        buffer[i] = i;
    }
    if (write(fd, buffer, write_size) == -1) {
        printf("Write before seek failed!\n");
        return -1;
    }
    if (lseek(fd, hole_size, SEEK_CUR) == -1) {
        printf("Seek failed!\n");
        return -1;
    }
    if (write(fd, buffer, write_size) == -1) {
        printf("Write after seek failed!\n");
        return -1;
    }

    close(fd);
    return 0;
}