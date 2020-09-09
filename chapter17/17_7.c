#include <fcntl.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "common.h"

int main() {
    int fd1 = open("f1", O_CREAT, 0644);
    if (fd1 < 0) {
        printf("Open failed!\n");
        return -1;
    }
    int fd2 = open("f1", O_CREAT, 0644);
    if (fd2 < 0) {
        printf("Open failed!\n");
        return -1;
    }
    int send_fds[2];
    send_fds[0] = fd1;
    send_fds[1] = fd2;

    int socket_fds[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, socket_fds) < 0) {
        printf("Create socket pair failed!\n");
        return -1;
    }

    printf("==================Send fds1==================\n");
    if (send_fds1(socket_fds[0], send_fds, 2) < 0) {
        printf("Send fds 1 failed!\n");
        return -1;
    }
    recv_fds1(socket_fds[1], 2);

    printf("==================Send fds2==================\n");
    if (send_fds2(socket_fds[0], send_fds, 2) < 0) {
        printf("Send fds 2 failed!\n");
        return -1;
    }
    recv_fds2(socket_fds[1], 2);



    return 0;
}