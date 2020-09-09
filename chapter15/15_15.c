#include <signal.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>

static volatile sig_atomic_t sig_flag1 = 0;
static volatile sig_atomic_t sig_flag2 = 0;
static sigset_t new_mask, old_mask, zero_mask;

static void sig_usr(int signum) {
    if (signum == SIGUSR1) {
        sig_flag1 = 1;
    } else if (signum == SIGUSR2) {
        sig_flag2 = 1;
    }
}

void TELL_WAIT(void) {
    if (signal(SIGUSR1, sig_usr) == SIG_ERR) {
        printf("Register SIGUSR1 failed!\n");
        return;
    }
    if (signal(SIGUSR2, sig_usr) == SIG_ERR) {
        printf("Register SIGUSR2 failed!\n");
        return;
    }
    sigemptyset(&zero_mask);
    sigemptyset(&new_mask);
    sigaddset(&new_mask, SIGUSR1);
    sigaddset(&new_mask, SIGUSR2);
    if (sigprocmask(SIG_BLOCK, &new_mask, &old_mask) < 0) {
        printf("Block signal failed!\n");
        return;
    }
}

void TELL_PARENT(pid_t pid) {
    kill(pid, SIGUSR2);
}

void WAIT_PARENT(void) {
    while (sig_flag1 == 0) {
        sigsuspend(&zero_mask);
    }
    sig_flag1 = 0;
    if (sigprocmask(SIG_SETMASK, &old_mask, NULL) < 0) {
        printf("Reset signal mask failed!\n");
        return;
    }
}

void TELL_CHILD(pid_t pid) {
    kill(pid, SIGUSR1);
}

void WAIT_CHILD(void) {
    while (sig_flag2 == 0) {
        sigsuspend(&zero_mask);
    }
    sig_flag2 = 0;
    if (sigprocmask(SIG_SETMASK, &old_mask, NULL) < 0) {
        printf("Reset signal mask failed!\n");
        return;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const key_t g_key = 0x12345678;
const size_t g_size = 4096;
#define NLOOPS 1000
#define SIZE sizeof(long)

int main() {
    // Create shm.
    int shmid = 0;
    if ((shmid = shmget(g_key, g_size, IPC_CREAT)) < 0) {
        printf("Create shm failed!\n");
        return -1;
    }

    // Attach shm.
    void* addr = shmat(shmid, 0, 0);
    if (addr == (void*)-1) {
        printf("Attach shm failed!\n");
        return -1;
    }
    TELL_WAIT();

    pid_t pid;
    if ((pid = fork()) < 0) {
        printf("Fork failed!\n");
        return -1;
    } else if (pid > 0) {
        // Parent.
        long* value = (long*)(addr);
        for (int i = 0; i < NLOOPS; i += 2) {
            printf("In parent, value:%ld\n", *value);
            *value += 1;
            TELL_CHILD(pid);
            WAIT_CHILD();
        }
    } else {
        // Child
        long* value = (long*)(addr);
        for (int i = 1; i < NLOOPS + 1; i += 2) {
            WAIT_PARENT();
            printf("In child, value:%ld\n", *value);
            *value += 1;
            TELL_PARENT(getppid());
        }
    }

    return 0;
}