#include <signal.h>
#include <stdio.h>

struct signal_info {
    int sig_num;
    char* message;
};

int main() {
    // Here should list all signals in figure 10.1, but I'm too lazy.
    struct signal_info signal_infos[] = {
        {SIGABRT, "abort"},
        {SIGALRM, "alarm"},
        {SIGBUS, "bus"},
        {SIGCHLD, "child"},
        {SIGINT, "interrupt"},
        {SIGKILL, "kill"},
        {SIGSTOP, "stop"},
    };

    // Fill signal mask for current process.
    sigset_t sigset;
    if (sigfillset(&sigset) < 0) {
        printf("Fill signal set failed!\n");
        return -1;
    }
    if (sigprocmask(SIG_SETMASK, &sigset, NULL) < 0) {
        printf("Set process signal mask failed!\n");
        return -1;
    }

    // Loop signal_info array to process all signals in signal mask.
    int num = sizeof(signal_infos) / sizeof(struct signal_info);
    for (int i = 0; i < num; ++i) {
        if (sigismember(&sigset, signal_infos[i].sig_num)) {
            printf("%s.\n", signal_infos[i].message);
        }
    }

    return 0;
}