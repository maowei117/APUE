#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "common.h"

#define MAXLINE 4096

int send_err(int fd, int errcode, const char* msg) {
    int n;
    if ((n = strlen(msg)) > 0) {
        if (write(fd, msg, n) != 0) { return -1; }
    }
    if (errcode >= 0) { errcode = -1; }
    if (send_fd(fd, errcode) < 0) { return -1; }
    return 0;
}

int send_fd(int fd, int fd_to_send) {
    struct msghdr msg;
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    char buf[2];
    struct iovec iov[1];
    iov[0].iov_base = buf;
    iov[0].iov_len = 2;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    char cmsgbuf[CMSG_SPACE(sizeof(int))];

    if (fd_to_send < 0) {
        msg.msg_control = NULL;
        msg.msg_controllen = 0;
        buf[1] = -fd_to_send;
        if (buf[1] == 0) { buf[1] = 1; }
    } else {
        msg.msg_control = cmsgbuf;
        msg.msg_controllen = sizeof(cmsgbuf);
        struct cmsghdr* cmsg = CMSG_FIRSTHDR(&msg);
        cmsg->cmsg_level = SOL_SOCKET;
        cmsg->cmsg_type = SCM_RIGHTS;
        cmsg->cmsg_len = CMSG_LEN(sizeof(int));
        *(int*)CMSG_DATA(cmsg) = fd_to_send;
        buf[1] = 0;
    }
    buf[0] = 0;

    int n = sendmsg(fd, &msg, 0);
    if ( n != 2) {
        printf("Send fd[%d] use by fd[%d] failed!\n", fd_to_send, fd);
        return -1;
    }
    return 0;
}

int recv_fd(int fd, ssize_t (*userfunc)(int, const void*, size_t)) {
    struct msghdr msg;
    char buf[MAXLINE];
    struct iovec iov[1];
    while (1) {
        iov[0].iov_base = buf;
        iov[0].iov_len = sizeof(buf);
        msg.msg_iov = iov;
        msg.msg_iovlen = 1;
        msg.msg_name = NULL;
        msg.msg_namelen = 0;
        char cmsgbuf[CMSG_SPACE(sizeof(int))];
        msg.msg_control = cmsgbuf;
        msg.msg_controllen = sizeof(cmsgbuf);
        struct cmsghdr* cmptr = CMSG_FIRSTHDR(&msg);

        int n = recvmsg(fd, &msg, 0);
        if (n < 0) { 
            printf("Receive message error!\n");
            return -1;
        } else if (n == 0) {
            printf("Connection closed by server.\n");
            return -1;
        }

        int status = -1;
        int new_fd = 0;
        for (char* ptr = buf; ptr < &buf[n];) {
            if (*ptr++ == 0) {
                if (ptr != &buf[n - 1]) {
                    printf("Message format error!\n");
                    break;
                }
                status = *ptr & 0xff;
                if (status == 0) {
                    if (msg.msg_controllen < CMSG_LEN(sizeof(int))) { printf("status = 0 but no fd!\n"); }
                    new_fd = *(int*)CMSG_DATA(cmptr);
                } else {
                    new_fd = -status;
                }
                n -= 2;
            }
        }

        if (n > 0 && (*userfunc)(STDERR_FILENO, buf, n) != n) { return -1; }
        if (status >= 0) { return new_fd; }
    }
    return 0;
}

int send_fds1(int fd, int* fds_to_send, int n) {
    struct msghdr msg;
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    char buf[2] = {0, 0};
    struct iovec iov[1];
    iov[0].iov_base = buf;
    iov[0].iov_len = 2;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    
    int cmsgbuf_len = CMSG_SPACE(sizeof(int) * n);
    char* cmsgbuf = (char*)(malloc(cmsgbuf_len));
    msg.msg_control = cmsgbuf;
    msg.msg_controllen = cmsgbuf_len;

    struct cmsghdr* cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(int));
    for (int i = 0; i < n; ++i) {
        ((int*)CMSG_DATA(cmsg))[i] = fds_to_send[i];
    }

    int num = sendmsg(fd, &msg, 0);
    if (num != 2) {
        printf("Send fds use by fd[%d] failed, num:%d!\n", fd, num);
        return -1;
    }
    return 0;
}

int recv_fds1(int fd, int n) {
    struct msghdr msg;
    char buf[MAXLINE];
    struct iovec iov[1];
    iov[0].iov_base = buf;
    iov[0].iov_len = sizeof(buf);
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    char cmsgbuf[CMSG_SPACE(sizeof(int) * n)];
    msg.msg_control = cmsgbuf;
    msg.msg_controllen = sizeof(cmsgbuf);
    struct cmsghdr* cmptr = CMSG_FIRSTHDR(&msg);

    int nun = recvmsg(fd, &msg, 0);
    if (nun < 0) { 
        printf("Receive message error!\n");
        return -1;
    } else if (nun == 0) {
        printf("Connection closed by server.\n");
        return -1;
    }

    int* fds = (int*)CMSG_DATA(cmptr);
    for (int i = 0; i < n; ++i) {
        printf("Receive fd:%d\n", fds[i]);
    }
    return 0;
}

int send_fds2(int fd, int* fds_to_send, int n) {
    struct msghdr msg;
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    char buf[2] = {0, 0};
    struct iovec iov[1];
    iov[0].iov_base = buf;
    iov[0].iov_len = 2;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    
    int cmsgbuf_len = n * CMSG_LEN(sizeof(int));
    char cmsgbuf[cmsgbuf_len]; 
    memset(cmsgbuf, 0, cmsgbuf_len);
    msg.msg_control = cmsgbuf;
    msg.msg_controllen = cmsgbuf_len;

    struct cmsghdr* cmsg = CMSG_FIRSTHDR(&msg);
    for (int i = 0; i < n; ++i) {
        cmsg->cmsg_level = SOL_SOCKET;
        cmsg->cmsg_type = SCM_RIGHTS;
        cmsg->cmsg_len = CMSG_LEN(sizeof(int));
        *(int*)CMSG_DATA(cmsg) = fds_to_send[i];
        cmsg = CMSG_NXTHDR(&msg, cmsg);
    }

    int num = sendmsg(fd, &msg, 0);
    if ( num != 2) {
        printf("Send fds use by fd[%d] failed, num:%d!\n", fd, num);
        perror("");
        return -1;
    }
    return 0;
}

int recv_fds2(int fd, int n) {
    struct msghdr msg;
    char buf[MAXLINE];
    struct iovec iov[1];
    while (1) {
        iov[0].iov_base = buf;
        iov[0].iov_len = sizeof(buf);
        msg.msg_iov = iov;
        msg.msg_iovlen = 1;
        msg.msg_name = NULL;
        msg.msg_namelen = 0;
        char cmsgbuf[CMSG_LEN(sizeof(int)) * n];
        memset(cmsgbuf, 0, CMSG_LEN(sizeof(int)) * n);
        msg.msg_control = cmsgbuf;
        msg.msg_controllen = sizeof(cmsgbuf);

        int n = recvmsg(fd, &msg, 0);
        if (n < 0) { 
            printf("Receive message error!\n");
            return -1;
        } else if (n == 0) {
            printf("Connection closed by server.\n");
            return -1;
        }

        for (struct cmsghdr* cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL; cmsg = CMSG_NXTHDR(&msg, cmsg)) {
            int fd = *(int*)CMSG_DATA(cmsg);
            printf("Receive fd:%d\n", fd);
        }
    }
    return 0;
}
