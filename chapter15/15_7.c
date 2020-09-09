#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

char* g_message = "hello world!\n";

int test_select_read() {
    printf("===========select read test============\n");
    int fds[2];
    if (pipe(fds) != 0) {
        printf("Create pipe failed!\n");
        return -1;
    }

    // Write pipe so that read fd has data to read.
    write(fds[1], g_message, strlen(g_message) + 1);
    sync();

    // Close write fd.
    close(fds[1]);

    // Use select.
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(fds[0], &readfds);
    int nfds = 128;
    int ret = select(nfds, &readfds, NULL, NULL, &timeout);
    if (ret == -1) {
        perror("Select error");
        return -1;
    } else if (ret == 0) {
        printf("No fd ready.\n");
        return 0;
    } else {
        if (FD_ISSET(fds[0], &readfds)) {
            printf("Write fd is marked readable.\n");
            char buffer[128];
            ssize_t read_size = read(fds[0], buffer, 128);
            printf("Read size:%ld msg:%s\n", read_size, buffer);
            return 0;
        } else {
            printf("Unexpected error!\n");
            return -1;
        }
    }
    return 0;
}

int test_select_write() {
    printf("===========select write test============\n");
    int fds[2];
    if (pipe(fds) != 0) {
        printf("Create pipe failed!\n");
        return -1;
    }

    // Close read fd.
    close(fds[0]);

    // Use select.
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    fd_set writefds;
    FD_ZERO(&writefds);
    FD_SET(fds[1], &writefds);
    int nfds = 128;
    int ret = select(nfds, NULL, &writefds, NULL, &timeout);
    if (ret == -1) {
        perror("Select error");
        return -1;
    } else if (ret == 0) {
        printf("No fd ready.\n");
        return 0;
    } else {
        if (FD_ISSET(fds[1], &writefds)) {
            printf("Write fd is marked writable.\n");
            ssize_t write_size = write(fds[1], g_message, strlen(g_message));
            printf("write_size:%ld\n", write_size);
            return 0;
        } else {
            printf("Unexpected error!\n");
            return -1;
        }
    }

    return 0;
}

int test_poll_read() {
        printf("===========poll read test============\n");
    int fds[2];
    if (pipe(fds) != 0) {
        printf("Create pipe failed!\n");
        return -1;
    }

    // Write pipe so that read fd has data to read.
    write(fds[1], g_message, strlen(g_message) + 1);
    sync();

    // Close write fd.
    close(fds[1]);

    // Use poll.
    struct pollfd read_poll_fd;
    read_poll_fd.fd = fds[0];
    read_poll_fd.events = POLLIN;

    int ret = poll(&read_poll_fd, 1, 1000);
    if (ret == -1) {
        perror("poll error");
        return -1;
    } else if (ret == 0) {
        printf("No fd ready.\n");
        return 0;
    } else {
        if (read_poll_fd.revents | POLL_IN != 0) {
            printf("Write fd is marked readable.\n");
            char buffer[128];
            ssize_t read_size = read(fds[0], buffer, 128);
            printf("Read size:%ld msg:%s\n", read_size, buffer);
            return 0;
        } else {
            printf("Unexpected error!\n");
            return -1;
        }
    }
    return 0;
}

int test_poll_write() {
    printf("===========poll write test============\n");
    int fds[2];
    if (pipe(fds) != 0) {
        printf("Create pipe failed!\n");
        return -1;
    }

    // Close read fd.
    close(fds[0]);

    // Use poll.
    struct pollfd read_poll_fd;
    read_poll_fd.fd = fds[1];
    read_poll_fd.events = POLL_OUT;
    int ret = poll(&read_poll_fd, 1, 1000);
    if (ret == -1) {
        perror("Poll error");
        return -1;
    } else if (ret == 0) {
        printf("No fd ready.\n");
        return 0;
    } else {
        if (read_poll_fd.revents | POLL_OUT != 0) {
            printf("Write fd is marked writable.\n");
            ssize_t write_size = write(fds[1], g_message, strlen(g_message));
            printf("write_size:%ld\n", write_size);
            return 0;
        } else {
            printf("Unexpected error!\n");
            return -1;
        }
    }
    return 0;
}

int test_read_fd() {
    test_select_read();
    test_poll_read();
    return 0;
}

int test_write_fd() {
    test_select_write();
    test_poll_write();
    return 0;
}

void signal_process(int signum) {
    printf("Receive signal:%d msg:%s\n", signum, strsignal(signum));
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <mode>\n", argv[0]);
        return 0;
    }

    // Register signal process.
    if (signal(SIGPIPE, signal_process) == SIG_ERR) {
        printf("Register SIGPIPE process error!\n");
        return -1;
    }

    int mode = atoi(argv[1]);
    switch(mode) {
        case 0:
            return test_read_fd();
        case 1:
            return test_write_fd();
        default:
            printf("Unsupported mode!\n");
            return -1;
    }
    return 0;
}