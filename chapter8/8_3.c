#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

void print_status(const siginfo_t* info) {
    if (info == NULL) { return; }
    int pid = info->si_pid;
    int code = info->si_code;
    int status = info->si_status;

    printf("pid[%d] -- ", pid);
    switch(code) {
        case CLD_EXITED:
            printf("Normal termination, exit status = %d\n", status);
            break;
        case CLD_KILLED:
            printf("Killed by signal, signal number = %d\n", WTERMSIG(status));
            break;
        case CLD_DUMPED:
            printf("Coredump, signal number:%d\n", WTERMSIG(status));
            break;
        case CLD_STOPPED:
            printf("Stopped, signal number:%d\n", WTERMSIG(status));
            break;
        case CLD_TRAPPED:
            printf("Trapped\n");
            break;
        case CLD_CONTINUED:
            printf("Continued\n");
            break;
        default:
            printf("Invalid code %d\n", code);
            return;
    }
}

int main() {
    pid_t pid = 0;
    int child_count = 0;
    int temp = 1;

    // exit
    if ((pid = fork()) < 0) {
        printf("Fork error!\n");
        return -1;
    } else if (pid == 0) {
        exit(7);
    }
    child_count++;

    // abort
    if ((pid = fork()) < 0) {
        printf("Fork error!\n");
        return -1;
    } else if (pid == 0) {
        abort();
    }
    child_count++;

    // divide 0
    if ((pid = fork()) < 0) {
        printf("Fork error!\n");
        return -1;
    } else if (pid == 0) {
        temp /= 0;
    }
    child_count++;

    // wait child process finish and print exit status.
    siginfo_t sig_info;
    
    while (child_count > 0) {
        if (waitid(P_ALL, 0, &sig_info, WEXITED | WSTOPPED | WCONTINUED) != 0) {
            printf("Wait error!\n");
            return -1;
        }
        print_status(&sig_info);
        child_count--;
    }

    return 0;
 }