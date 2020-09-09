#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>

int ptym_open(char *slave_name, int slave_len);
int ptys_open(char* slave_name);
pid_t pty_fork(int *ptyfdm, char* slave_name, int slave_len,
               const struct termios* slave_termios, const struct winsize* slave_winsize);
void loop(int ptym, int ignoreeof);

void set_noecho(int fd);

int main(int argc, char* argv[]) {
    // Get options.
    int opt = -1;
    char* driver = NULL;
    int noecho = 0;
    int ignoreeof = 0;
    int interactive = isatty(STDIN_FILENO);
    int verbose = 0;
    opterr = 0;  // Disable getopt() error msg.
    while ((opt = getopt(argc, argv, "+d:einv")) != -1) {
        switch(opt) {
            case 'd':
                driver = optarg;
                break;
            case 'e':
                noecho = 1;
                break;
            case 'i':
                ignoreeof = 1;
                break;
            case 'n':
                interactive = 0;
                break;
            case 'v':
                verbose = 1;
                break;
            default:
                printf("Invalid options:-%c", optopt);
        }
    }

    pid_t pid;
    int fd_master;
    char slave_name[256];
    if (interactive) {
        struct termios origin_termios;
        if (tcgetattr(STDIN_FILENO, &origin_termios) < 0) {
            printf("Tcgetattr error on stdin.\n");
            return -1;
        }
        struct winsize size;
        if (ioctl(STDIN_FILENO, TIOCGWINSZ, (char*)&size) < 0) {
            printf("Get windows size failed!\n");
            return -1;
        }
        pid = pty_fork(&fd_master, slave_name, sizeof(slave_name), &origin_termios, &size);
    } else {
        pid = pty_fork(&fd_master, slave_name, sizeof(slave_name), NULL, NULL);
    }

    if (pid < 0) {
        printf("Fork error!\n");
    } else if (pid == 0) {
        // Child
        if (noecho) {
            set_noecho(STDIN_FILENO);
        }

        if (execvp(argv[optind], &argv[optind]) < 0) {
            printf("Exec %s failed!\n", argv[optind]);
            return -1;
        }
    }

    // Parent.
    if (verbose) {
        printf("slave name = %s driver = %s\n", slave_name, driver);
    }

    loop(fd_master, ignoreeof);
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

static volatile sig_atomic_t sigcaught = 0;
static void* handler(int signum) {
    sigcaught = 1;
}

/*
void loop(int ptym, int ignoreeof) {
    int num = 2;
    struct pollfd pfds[num];
    pfds[0].fd = ptym;
    pfds[0].events = POLLIN;
    pfds[1].fd = STDIN_FILENO;
    pfds[1].events = POLLIN;
    int write_fds[num];
    write_fds[0] = STDOUT_FILENO;
    write_fds[1] = ptym;

    const int bufsize = 4096;
    char buffer[bufsize];

    while (1) {
        int n = poll(pfds, num, -1);
        if (n < 0) { break; }

        for (int i = 0; i < num; ++i) {
            if (pfds[i].revents & POLLIN) {
                // Read.
                int rfd = pfds[i].fd;
                int nread = read(rfd, buffer, bufsize);
                if (nread <= 0) { return; }

                int wfd = write_fds[i];
                if (write(wfd, buffer, nread) != nread) { return; }
                pfds[i].revents = 0;
            } else if (pfds[i].revents & POLLHUP) { return; }
        }
    }
}
*/

void loop(int ptym, int ignoreeof) {
    const int bufsize = 4096;
    char buffer[bufsize];

    pid_t pid = fork();
    if (pid < 0) {
        printf("Fork error!\n");
        return;
    } else if (pid == 0) {
        // Child.
        while (1) {
            int nread = read(STDIN_FILENO, buffer, bufsize);
            if (nread < 0) {
                printf("Read error!\n");
            } else if (nread == 0) {
                break;
            }
            if (write(ptym, buffer, nread) != nread) {
                printf("Write error!\n");
            }
 
        }
        if (ignoreeof == 0) {
            kill(getppid(), SIGTERM);
        }
        exit(0);
    }

    // Parent.
    signal(SIGTERM, handler);

    while (1) {
        int nread = read(ptym, buffer, bufsize);
        if (nread <= 0) { break; }
        if (write(STDOUT_FILENO, buffer, nread) != nread) {
            printf("Write error!\n");
        }
    }

    if (sigcaught == 0) {
        kill(pid, SIGTERM);
    }
}

void set_noecho(int fd) {
    struct termios stermios;
    if (tcgetattr(fd, &stermios) < 0) {
        printf("Get attr failed!\n");
        return;
    }

    stermios.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);
    stermios.c_oflag &= ~(ONLCR);
    if (tcsetattr(fd, TCSANOW, &stermios) < 0) {
        printf("Set attr failed!\n");
        return;
    }
}

