#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

int ptym_open(char *slave_name, int slave_len);
int ptys_open(char* slave_name);
pid_t pty_fork(int *ptyfdm, char* slave_name, int slave_len,
               const struct termios* slave_termios, const struct winsize* slave_winsize);

int main(int argc, char* argv[]) {
    int fdm = -1;
    char slave_name[256];
    pid_t pid = pty_fork(&fdm, slave_name, 256, NULL, NULL);
    if (pid < 0) {
        printf("Fork failed!\n");
        return -1;
    } else if (pid == 0) {
        // Child.
        const char* callee = "19_9_callee";
        char *newargv[] = {callee, NULL};
        if (execv(callee, newargv) < 0) {
            printf("Exec %s failed!\n", callee);
            return -1;
        }
    }
    
    // Parent.
    int max_row = 100;
    int max_col = 100;
    char buffer[4096];
    while (1) {
        sleep(1);

        // Send SIGTERM to child process, not support in my system.
        /*
        int signum = SIGTERM;
        if (ioctl(fdm, TIOCSIG, &signum) < 0) {
            printf("Send signal to child failed!\n");
            perror("");
            return -1;
        }
        */

        // Set window size.
        struct winsize window_size;
        window_size.ws_row = random() % max_row;
        window_size.ws_col = random() % max_col;
        if (ioctl(fdm, TIOCSWINSZ, &window_size) < 0) {
            printf("Change windows size failed!\n");
            return -1;
        }
        printf("Change winsize in parent, row:%d col:%d\n", window_size.ws_row, window_size.ws_col);

        // Read message from child.
        int n = read(fdm, buffer, 4096);
        printf("%s\n", buffer);
    }

    return 0;
}

int ptym_open(char *slave_name, int slave_len) {
    // Get master fd.
    int fd = posix_openpt(O_RDWR);
    if ( fd < 0) {
        printf("Open ptm failed!\n");
        return -1;
    }

    // Change slave permission.
    if (grantpt(fd) < 0) {
        printf("Grantpt failed!\n");
        return -1;
    }

    // Unlock slave device.
    if (unlockpt(fd) < 0) {
        printf("Unlock slave device failed!\n");
        return -1;
    }

    if (ptsname_r(fd, slave_name, slave_len) != 0) {
        printf("Get slave name failed!\n");
        return -1;
    }
    return fd;
}

int ptys_open(char* slave_name) {
    if (slave_name == NULL) { return -1; }
    int fd = open(slave_name, O_RDWR);
    if (fd < 0) {
        printf("Open slave %s failed!\n", slave_name);
        return -1;
    }
    return fd;
}

pid_t pty_fork(int *ptyfdm, char* slave_name, int slave_len,
               const struct termios* slave_termios, const struct winsize* slave_winsize) {
    // Open pt master.
    char pts_name[128];
    int fdm = ptym_open(pts_name, sizeof(pts_name));
    if (fdm < 0) {
        printf("Open pty master failed!\n");
        return -1;
    }

    // Assign slave name.
    strncpy(slave_name, pts_name, slave_len);

    pid_t pid = fork();
    if (pid < 0) {
        printf("Fork failed!\n");
        return -1;
    } else if (pid == 0) {
        // Child.
        // Create a new session, make child process detach from control terminal.
        if (setsid() < 0) {
            printf("Create session failed!\n");
            return -1;
        }

        // Acquire control terminal while opening slave fd.
        int fds = ptys_open(pts_name);
        if (fds < 0) {
            printf("Open slave pty failed!\n");
            return -1;
        }

        // Set slave's termios and window size.
        if (slave_termios != NULL) {
            if (tcsetattr(fds, TCSANOW, slave_termios) < 0) {
                printf("Set slave's termios failed!\n");
                return -1;
            }
        }

        if (slave_winsize != NULL) {
            if (ioctl(fds, TIOCSWINSZ, slave_winsize) < 0) {
                printf("Set slave window size failed!\n");
                return -1;
            }
        }

        // Redirect stdin/stdout/stderr to slave fd.
        if (dup2(fds, STDIN_FILENO) != STDIN_FILENO) {
            printf("Dup fds to stdin failed!\n");
            return -1;
        }
        if (dup2(fds, STDOUT_FILENO) != STDOUT_FILENO) {
            printf("Dup fds to stdout failed!\n");
            return -1;
        }
        if (dup2(fds, STDERR_FILENO) != STDERR_FILENO) {
            printf("Dup fds to stderr failed!\n");
            return -1;
        }

        // Close fds if not used.
        if (fds != STDIN_FILENO && fds != STDOUT_FILENO && fds != STDERR_FILENO) {
            close(fds);
        }
        return 0;
    } else {
        // Parent.
        *ptyfdm = fdm;
        return pid;
    }

    return 0;
}