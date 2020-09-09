#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#define SEEK_POS 20

int main() {
    const char* file = "test.txt";
    int fd = open(file, O_RDWR | O_APPEND);
    if (fd < 0) {
        printf("Open file failed!\n");
        return -1;
    }

    lseek(fd, SEEK_POS, SEEK_SET);

    // Read
    int buf_size = 20;
    char read_buf[buf_size + 1];
    read_buf[buf_size] = '\0';
    ssize_t n = read(fd, read_buf, buf_size);
    printf("Read:%s %ldbytes\n", read_buf, n);

    lseek(fd, 0, SEEK_SET);

    // Write
    const char* write_msg = "WRITE";
    if (write(fd, write_msg, 5) < 0) { return -1; }
    return 0;
}