#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

static const char* k_outputPath = "bigfile";
static const int k_bufferSize = 1 << 30;  // 1GB

static void handler(int signum) {
    printf("Catch signal:%d\n", signum);
}

int main() {
    // Create big buffer.
    char* buffer = (char*)malloc(k_bufferSize);
    if (buffer == NULL) {
        printf("malloc failed!\n");
        return -1;
    } 

    // Initialize buffer.
    for (int i = 0; i < k_bufferSize; ++i) {
        buffer[i] = i % 10;
    }

    // Register alarm handler.
    struct sigaction act;
    act.sa_flags = 0;
    act.sa_handler = handler;
    if (sigaction(SIGALRM, &act, NULL) < 0) {
        printf("Register alram signal handler failed!\n");
        return -1;
    }

    // Write file.
    alarm(1);
    FILE* fp = fopen(k_outputPath, "w");
    if (fp == NULL) {
        printf("Craete file failed!\n");
        return -1;
    }
    if (fwrite(buffer, k_bufferSize, 1, fp) != 1) {
        printf("Write failed!\n");
    }
    printf("Write success!\n");

    return 0;
}