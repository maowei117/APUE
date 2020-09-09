#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "common.h"

static const char* k_fileName = "temp_file";

#define OFFSET1 12345
#define OFFSET2 54321

ssize_t func(int fd, const void* msg, size_t msg_len) {
    printf("%s\n", (char*)msg);
    return 0;
}

int main() {
    // Create unix socket.
    int fds[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) < 0) {
        printf("Create socket pair failed!\n");
        return -1;
    }
    printf("fds[0]:%d fds[1]:%d.\n", fds[0], fds[1]);

    pid_t pid = fork();
    if (pid == 0) {
        // Child.
        int fd = open(k_fileName, O_CREAT | O_RDWR, 0666);
        if (fd < 0) {
            printf("Open file failed!\n");
            return -1;
        }

        // Send fd to parent.
        if (send_fd(fds[0], fd) != 0) {
            printf("Send fd:%d failed!\n", fd);
            return -1;
        }

        // seek to pos1.
        printf("Child change offset to %d.\n", OFFSET1);
        off_t offset1 = lseek(fd, OFFSET1, SEEK_SET);
        printf("Child, offset1:%ld.\n", offset1);

        sleep(2);  // Wait parent.

        off_t offset2 = lseek(fd, 0, SEEK_CUR);
        printf("Child, offset2:%ld.\n", offset2);
    } else if (pid > 0) {
        // Parent.
        sleep(1);  // Wait child.

        // Receive fd
        int fd = recv_fd(fds[1], func);
        printf("receive fd:%d\n", fd);

        off_t offset1 = lseek(fd, 0, SEEK_CUR);
        printf("Parent, offset1:%ld.\n", offset1);

        printf("Parent change offset to %d.\n", OFFSET2);
        off_t offset2 = lseek(fd, OFFSET2, SEEK_SET);
        printf("Parent, offset2:%ld.\n", offset2);

        waitpid(pid, NULL, 0);
    } else {
        printf("Fork failed!\n");
    }
    return 0;
}