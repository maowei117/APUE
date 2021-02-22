#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

static volatile sig_atomic_t sig_flag1 = 1;
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

/////////////////////////////////////////////////////////////////

#define RW_PERMISSION (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)

static const char* k_filePath = "./counter.txt";
static const int k_maxCounter = 100;

int fetch_add_counter(int fd, const char* who) {
    int counter = 0;
    if (pread(fd, &counter, sizeof(counter), 0) != sizeof(counter)) {
        printf("Read counter failed!\n");
        return -1;
    }
    counter++;
    printf("%s add counter, current:%d\n", who, counter);
    if (pwrite(fd, &counter, sizeof(counter), 0) != sizeof(counter)) {
        printf("Write counter failed!\n");
        return -1;
    }
    sync();
    return counter;
}

int main() {
    int fd = open(k_filePath, O_CREAT| O_TRUNC | O_RDWR, RW_PERMISSION);
    if (fd < 0) {
        printf("Open file failed!\n");
        return -1;
    }

    int counter = 0;
    if (write(fd, &counter, sizeof(counter)) != sizeof(counter)) {
        printf("Write counter to file failed!\n");
        return -1;
    }
    sync();

    TELL_WAIT();

    pid_t pid = fork();
    if (pid == 0) {
        // Child
        int parent_pid = getppid();
        while (1) {
            WAIT_PARENT();
            if (fetch_add_counter(fd, "child") < 0) {
                printf("Child fetch add counter failed!\n");
                return -1;
            }
            TELL_PARENT(parent_pid);
            if (counter >= k_maxCounter) { break; }
        }
    } else if (pid > 0) {
        // Parent
        int child_pid = pid;
        while (1) {
            WAIT_CHILD();
            int counter = fetch_add_counter(fd, "parent");
            if (counter < 0) {
                printf("Parent fetch add counter failed!\n");
                return -1;
            }
            TELL_CHILD(child_pid);

            if (counter >= k_maxCounter) { break; }
        }
    } else {
        printf("Fork failed!\n");
        return -1;
    }

    close(fd);

    return 0;
}