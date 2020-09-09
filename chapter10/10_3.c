#include <stdio.h>
#include <signal.h>
#include <unistd.h>


void alarm_handler(int signum) {
    printf("alarm\n");
}

void intr_handler(int signum) {
    printf("intr handler start!\n");
    alarm(2);
    pause();
    printf("intr handler finished!\n");
}

int main() {
    // Use alarm to interrupt processing signal.
    signal(SIGALRM, alarm_handler);
    signal(SIGINT, intr_handler);
    pause();
    return 0;
}