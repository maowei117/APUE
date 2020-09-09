#include <stdio.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

void* handler(int signum) {
    switch (signum) {
        case SIGTERM:
            printf("Receive SIGTERM\n");
            break;
        case SIGWINCH:
            printf("Receive SIGWINCH\n");
            
            // Print window size.
            struct winsize window_size;
            if (ioctl(STDIN_FILENO, TIOCGWINSZ, &window_size) < 0) {
                printf("Change windows size failed!\n");
                return -1;
            }
            printf("Window row:%d col:%d\n", window_size.ws_row, window_size.ws_col);
            break;
        default:
            printf("Receive unknown signal %d\n", signum);
            break;
    }
}

int main(int argc, char* argv[]) {
    printf("%s running\n", argv[0]);
    // Register signal.
    signal(SIGTERM, handler);
    signal(SIGWINCH, handler);

    while (1) {
        sleep(1);
    }
    printf("Child quit\n");
    return 0;
}