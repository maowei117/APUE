#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_SIZE 4096

static const char* k_localIp = "127.0.0.1";
static const int k_maxConnections = 128;
static const int k_maxEvents = 128;

int main(int argc, char* argv[]) {
    if (argc < 4) {
        printf("Usage: %s <port_base> <port_interval> <num>\n", argv[0]);
        return -1;
    }

    int port_base = atoi(argv[1]);
    int port_interval = atoi(argv[2]);
    int num = atoi(argv[3]);
    if (port_base < 0 || port_interval < 0 || num <= 0) {
        printf("Error params!\n");
        return -1;
    }

    // Initialize epoll.
    int epfd = epoll_create1(0);
    if (epfd < 0) {
        printf("Create epoll failed!\n");
        return -1;
    }

    for (int i = 0; i < num; ++i) {
        // Create sockets.
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0) {
            printf("Create socket failed!\n");
            return -1;
        }

        // Bind address.
        struct sockaddr_in ipv4_addr;
        ipv4_addr.sin_family = AF_INET;
        ipv4_addr.sin_port = port_base + i * port_interval;
        if (inet_pton(AF_INET, k_localIp, &(ipv4_addr.sin_addr.s_addr)) != 1) {
            printf("Transform ip to address failed!\n");
            return -1;
        }
        if (bind(fd, (struct sockaddr*)&ipv4_addr, sizeof(struct sockaddr_in)) < 0) {
            printf("Bind address failed!\n");
            return -1;
        }

        // Listen on socket.
        if (listen(fd, k_maxConnections) < 0) {
            printf("Listen socket failed!\n");
            return -1;
        }

        // Add to epoll.
        struct epoll_event ev;
        ev.data.fd = fd;
        ev.events = EPOLLIN;
        if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) < 0) {
            printf("Add fd to epoll failed!\n");
            return -1;
        }
    }

    // Use epoll to process requests.
    struct epoll_event events[k_maxEvents];
    while (1) {
        int nfds = epoll_wait(epfd, events, k_maxEvents, -1);
        if (nfds < 0) {
            printf("Epoll wait failed!\n");
            return -1;
        }

        struct sockaddr_in peer_addr;
        socklen_t len = 0;
        for (int i = 0; i < nfds; ++i) {
            int conn_fd = accept(events[i].data.fd, (struct sockaddr*)&peer_addr, &len);
            if (conn_fd < 0) {
                printf("Accept failed!\n");
                return -1;
            } 

            char buf[128];
            printf("Connected to %s:%d\n",
                   inet_ntop(AF_INET, &(peer_addr.sin_addr.s_addr), buf, 128), peer_addr.sin_port);

            // Receive cmd from client.
            char read_buffer[BUFFER_SIZE];
            if (recv(conn_fd, read_buffer, BUFFER_SIZE, 0) < 0) {
                printf("Receive cmd failed!\n");
                return -1;
            }

            char* cmd = NULL;
            if (strcmp(read_buffer, "uptime") == 0) {
                cmd = "/usr/bin/uptime";
            } else if (strcmp(read_buffer, "process_count") == 0) {
                cmd = "ps -ef | wc -l";
            } else {
                cmd = "ls";
            }

            // Call uptime.
            char send_buffer[BUFFER_SIZE];
            FILE* fp = popen(cmd, "r");
            if (fp == NULL) {
                sprintf(send_buffer, "Call <uptime> failed, error:%s!\n", strerror(errno));
                send(conn_fd, send_buffer, strlen(send_buffer), 0);
            } else {
                while (fgets(send_buffer, BUFFER_SIZE, fp) != NULL) {
                    send(conn_fd, send_buffer, strlen(send_buffer), 0);
                }
                pclose(fp);
            }
            close(conn_fd);
        }

    }

    return 0;
}