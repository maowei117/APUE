#include <stdio.h>
#include <unistd.h>
#include <termios.h>

int tty_raw(int fd) {
    struct termios buf;
    if (tcgetattr(fd, &buf) < 0) {
        printf("Get terminal attr failed!\n");
        return -1;
    }

    buf.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    buf.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    buf.c_cflag &= ~(CSIZE | PARENB);
    buf.c_cflag |= CS8;
    buf.c_oflag &= ~(OPOST);
    buf.c_cc[VMIN] = 1;
    buf.c_cc[VTIME] = 0;

    if (tcsetattr(fd, TCSAFLUSH, &buf) < 0) {
        return -1;
    }
    return 0;
}

int main() {
    tty_raw(STDIN_FILENO);
    return 0;
}