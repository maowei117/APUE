#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <sys/resource.h>
#include <unistd.h>

#define BUFSIZE 100
#define MAX_FILE_SIZE 1

void signal_intr(int signum) {
    if (signum == SIGXFSZ) {
        printf("Process SIGXFSZ\n");
    }
    return;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: %s <input_file> <output_file>\n", argv[0]);
        return -1;
    }

    // Register signal.
    struct sigaction action;
    action.sa_flags = 0;
    action.sa_handler = signal_intr;
    if (sigaction(SIGXFSZ, &action, NULL) < 0) {
        printf("Register signal failed!\n");
        return -1;
    }

    // Limit file size.
    struct rlimit limit;
    limit.rlim_cur = MAX_FILE_SIZE;
    limit.rlim_max = MAX_FILE_SIZE;
    if (setrlimit(RLIMIT_FSIZE, &limit) < 0) {
        printf("Limit resource failed!\n");
        return -1;
    }

    struct rlimit cur_limit;
    if (getrlimit(RLIMIT_FSIZE, &cur_limit) < 0) {
        printf("Get current limit failed!\n");
        return -1;
    }
    printf("Current limit soft:%ld, max:%ld\n", cur_limit.rlim_cur, cur_limit.rlim_max);

    int fd_in = open(argv[1], O_RDONLY);
    int fd_out = open(argv[2], O_WRONLY | O_CREAT , 0644);
    int n;
    char buf[BUFSIZE];
    while ((n = read(fd_in, buf, BUFSIZE)) == BUFSIZE) {
        if (write(fd_out, buf, n) != BUFSIZE) {
            printf("Not fully write, write size:%d\n", n);
            break;
        }
    }

    printf("Now write again!\n");
    if (write(fd_out, buf + n, BUFSIZE - n) < 0) {
        perror("Write to file failed!");
    }
    return 0;
}