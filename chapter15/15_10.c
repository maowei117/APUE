#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

void sig_process(int signum) {
    printf("%s\n", strsignal(signum));
}

const char* g_message = "hello";

int main() {
    if (signal(SIGPIPE, sig_process) < 0) {
        printf("Register SIGPIPE process failed!\n");
        return -1;
    }

    // Create FIFO if not exists.
    const char* fifo_name = "./tempfifo";
    struct stat statbuf;
    if (stat(fifo_name, &statbuf) != 0) {
        if (mkfifo(fifo_name, 0666) < 0) {
            printf("Make fifo failed!\n");
            return -1;
        }
    }

    // Open FIFO.
    int fd = open(fifo_name, O_RDWR | O_NONBLOCK);
    if (fd < 0) {
        printf("Open FIFO failed!\n");
        return -1;
    }
    printf("Open success, fd:%d\n", fd);

    // Write FIFO.
    ssize_t size = write(fd, g_message, strlen(g_message));
    printf("Write fifo success, size:%ld\n", size);

    // Read FIFO
    char buffer[128];
    ssize_t read_size = read(fd, buffer, 128);
    printf("Read success, size:%ld msg:%s\n", read_size, buffer);
    
    return 0;
}