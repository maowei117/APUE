#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>


void daemonize(const char* cmd) {
    umask(0);

    struct rlimit rl;
    if (getrlimit(RLIMIT_NOFILE, &rl) < 0) {
        printf("Can't get file limit!\n");
        return;
    }

    pid_t pid = 0;
    if ((pid = fork()) < 0) {
        printf("Fork error!\n");
        return;
    } else if (pid != 0) {
        exit(0);
    }

    setsid();

    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGHUP, &sa, NULL) < 0) {
        printf("Ignore SIGHUP error!\n");
        return;
    }
    if ((pid = fork()) < 0) {
        printf("Fork error!\n");
    } else if (pid != 0) {
        exit(0);
    }

    if (chdir("/") < 0) {
        printf("Change dir failed!\n");
        return;
    }

    if (rl.rlim_max == RLIM_INFINITY) {
        rl.rlim_max = 1024;
    }
    for (int i = 0; i < rl.rlim_max; ++i) {
        close(i);
    }

    int fd0 = open("/dev/null", O_RDWR);
    int fd1 = dup(0);
    int fd2 = dup(0);

    openlog(cmd, LOG_CONS, LOG_DAEMON);
    if (fd0 != 0 || fd1 != 1 || fd2 != 2) {
        syslog(LOG_ERR, "Unexpected fd %d %d %d\n", fd0, fd1, fd2);
        exit(1);
    }
}

int main() {
    daemonize("tommao");
    char* login = getlogin();
    syslog(LOG_DEBUG, "Login:%s", login);
    pause();
}