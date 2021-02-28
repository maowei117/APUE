#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void process_abort(int signum) {
    printf("Accept SIGABRT!\n");
}

int main() {
    struct sigaction act;
    act.sa_flags = 0;
    act.sa_handler = process_abort;
    sigaction(SIGABRT, &act, NULL);

    printf("Before abort()\n");
    abort();
    printf("After abort()\n");
    while (1) {
        pause();
    }
    return 0;
}