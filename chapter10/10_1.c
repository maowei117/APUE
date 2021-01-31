#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static void handler(int signum) {
    printf("Receive signal:%d, message:%s\n", signum, strsignal(signum));
    psignal(signum, "");
}

int main() {
    signal(SIGUSR1, handler);
    signal(SIGUSR2, handler);

    // If not loop to pause, process will return when catch first signal.
    pause();
    return 0;
}