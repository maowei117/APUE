#include <signal.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

void print_infos(const char* prefix) {
    printf("%s:\tpid:%d\tppid:%d\tsid:%d\tgid:%d\ttpgrp:%d\n",
           prefix, getpid(), getppid(), getsid(0), getpgid(0), tcgetpgrp(STDIN_FILENO));
}

static void sig_ttin(int signo) {
    printf("Receive SIG_TTIN\n");
}

int main() {
    print_infos("Before fork");
    // Create child thread.
    pid_t pid = fork();
    if (pid > 0) {
        print_infos("Parent");

        int status = 0;
        wait(&status);
    } else if (pid == 0) {        
        print_infos("Child");

        signal(SIGTTIN, sig_ttin);

        // Create new session.
        if (setsid() == -1) {
            printf("setsid failed!\n");
            return -1;
        }
        print_infos("Child new session");

        // Try to read from stdin.
        char c;
        if (read(STDIN_FILENO, &c, 1) != 1) {
            printf("Read error!\n");
            return -1;
        }
        printf("Read ok: %c\n", c);
    } else {
        printf("Fork error!\n");
        return -1;
    }

    return 0;
}