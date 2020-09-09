#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int file_lock(int fd, int cmd, int type) {
    struct flock lock;
    lock.l_type = type;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_whence = SEEK_SET;
    return fcntl(fd, cmd, &lock);
} 

void sig_handler(int signum) {
    return;
}

int main() {
    signal(SIGINT, sig_handler);

    const char* file = "templock";
    int fd = open(file, O_RDWR | O_CREAT, 0666);
    if (fd < 0) {
        printf("Open file %s failed\n", file);
        return -1;
    }

    int pid1;
    if ((pid1 = fork()) < 0) {
        printf("Fork error!\n");
        return -1;
    } else if (pid1 == 0) {
        // Child1, set file a read lock.
        if (file_lock(fd, F_SETLK, F_RDLCK) < 0) {
            printf("Child1 set read lock failed!\n");
            return -1;
        } else {
            printf("Child1 get file lock success.\n");
        }
        pause();
        printf("Child1 exit.\n");
        exit(0);
    } 

    sleep(2);

    int pid2;
    if ((pid2 = fork()) < 0) {
        printf("Fork error!\n");
        return -1;
    } else if (pid2 == 0) {
        // Child2, set file a read lock.
        if (file_lock(fd, F_SETLK, F_RDLCK) < 0) {
            printf("Child2 set read lock failed!\n");
            return -1;
        } else {
            printf("Child2 get file lock success.\n");
        }
        pause();
        printf("Child2 exit.\n");
        exit(0);
    } 

    sleep(2);

    int pid3;
    if ((pid3 = fork()) < 0) {
        printf("Fork error!\n");
        return -1;
    } else if (pid3 == 0) {
        // Child3, set file a write lock.
        if (file_lock(fd, F_SETLK, F_WRLCK) < 0) {
            printf("Child3 set non-block write lock failed!\n");
        } else {
            printf("Child3 set non-block write lock success.\n");
        }

        // Set a block write lock.
        printf("Try to set a block write lock in child3.\n");
        if (file_lock(fd, F_SETLKW, F_WRLCK) < 0) {
            printf("Child3 set block write lock failed!\n");
        } else {
            printf("Child3 set block write lock success!\n");
        }
        pause();
        printf("Child3 exit.\n");
        exit(0);
    } 

    sleep(2);

    // Set a read lock after block write lock.
    if (file_lock(fd, F_SETLK, F_RDLCK) < 0) {
        printf("Parent set read lock failed!\n");
    } else {
        printf("Parent set read lock success.\n");
    }

    // Unlock all read lock and see if child3 get write lock.
    if (file_lock(fd, F_SETLK, F_UNLCK) < 0) {
        printf("Parent unlock failed:%s!\n", strerror(errno));
    } else {
        printf("Parent unlock success.\n");
    }
    printf("Kill child1.\n");
    kill(pid1, SIGINT);
    printf("Kill child2.\n");
    kill(pid2, SIGINT);


    sleep(2);
    return 0;
}