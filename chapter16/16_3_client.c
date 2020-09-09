#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#define BUFFER_SIZE 4096

int main(int argc, char* argv[]) {
    if (argc < 4) {
        printf("Usage: %s <ip> <port> <cmd>\n", argv[0]);
        return -1;
    }

    // Create socket.
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        printf("Create socket failed!\n");
        return -1;
    }

    int port = atoi(argv[2]);
    
    // Connect to server.
    struct sockaddr_in ipv4_addr;
    ipv4_addr.sin_family = AF_INET;
    ipv4_addr.sin_port = port;
    if (inet_pton(AF_INET, argv[1], &(ipv4_addr.sin_addr.s_addr)) != 1) {
        printf("Transform ip to address failed!\n");
        return -1;
    }
    if (connect(fd, (struct sockaddr*)&ipv4_addr, sizeof(struct sockaddr_in)) < 0) {
        printf("Connect failed!\n");
        return -1;
    }

    // Send request to server.
    ssize_t send_len = strlen(argv[3]);
    if (send(fd, argv[3], send_len, 0) != send_len) {
        printf("Send request to server failed!\n");
        return -1;
    }
    printf("Send cmd:%s to server.\n", argv[3]);

    // Receive response from server.
    char read_buffer[BUFFER_SIZE];
    char write_buffer[BUFFER_SIZE];
    int n = 0;
    int pos = 0;
    while ((n = recv(fd, read_buffer, BUFFER_SIZE, 0)) > 0) {
        memcpy(write_buffer + pos, read_buffer, n);
        pos += n;
    }
    write_buffer[pos] = '\0';
    printf("%s\n", write_buffer);
    return 0;
}