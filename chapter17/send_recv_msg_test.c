#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <string.h>
#include <unistd.h>

int main() {
    // Create Unix domain socket.
    int fd[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fd) < 0) {
        printf("Create unix socket failed!\n");
        return -1;
    }

    // Write str message to fd[0].
    const char* message = "hello";
    if (write(fd[0], message, strlen(message)) != strlen(message)) {
        printf("Write message failed!\n");
        return -1;
    }

    // Use sendmsg to send message.
    struct msghdr msg;
    msg.msg_name = NULL;
    msg.msg_namelen = 0;

    char* str0 = "iov0_message";
    char* str1 = "iov1_message";
    struct iovec iov[2];
    iov[0].iov_base = str0;
    iov[0].iov_len = strlen(str0);
    iov[1].iov_base = str1;
    iov[1].iov_len = strlen(str1);
    msg.msg_iov = iov;
    msg.msg_iovlen = 2;

    char buf[CMSG_SPACE(sizeof(int))];
    msg.msg_control = buf;
    msg.msg_controllen = sizeof(buf);
    struct cmsghdr* cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(int));
    *(int*)CMSG_DATA(cmsg) = fd[0];

    ssize_t n = sendmsg(fd[0], &msg, 0);
    if (n <= 0) {
        printf("Send msg failed!\n");
        return -1;
    }
    printf("Send msg success, total:%zu bytes.\n", n);

    // Recv message.
    struct msghdr read_msg;
    read_msg.msg_name = NULL;
    read_msg.msg_namelen = 0;

    const int buffer_len = 4096;
    char* read_buffer = (char*)(malloc(buffer_len));
    struct iovec read_iov;
    read_iov.iov_base = read_buffer;
    read_iov.iov_len = buffer_len;
    read_msg.msg_iov = &read_iov;
    read_msg.msg_iovlen = 1;

    char* msg_buffer = (char*)(malloc(buffer_len));
    read_msg.msg_controllen = buffer_len;
    read_msg.msg_control = msg_buffer;

    ssize_t m = recvmsg(fd[1], &read_msg, 0);
    printf("Recv message, total_len:%zu message:%s\n", m, (char*)read_iov.iov_base);

    return 0;
}