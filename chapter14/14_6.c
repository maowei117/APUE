#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int TELL_WAIT() {
    // Create file.
    const char* temp_file = "tempfile";
    int fd = open(temp_file, O_RDWR | O_CREAT, 0666);
    if (fd < 0) {
        printf("Open file failed!\n");
        exit(-1);
    }
    return fd;
}

void WAIT_PARENT(int fd) {
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_whence = SEEK_SET;
    printf("Child try to get lock\n");
    if (fcntl(fd, F_SETLKW, &lock) < 0) {
        printf("Wait parent, lock error!\n");
    }
    printf("Child get lock\n");
}

void WAIT_CHILD(int fd) {
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_whence = SEEK_SET;
    printf("Parent try to get lock\n");
    if (fcntl(fd, F_SETLKW, &lock) < 0) {
        printf("Wait child, lock error!\n");
    }
    printf("Parent get lock\n");
}

void TELL_PARENT(int fd) {
    struct flock lock;
    lock.l_type = F_UNLCK;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_whence = SEEK_SET;
    if (fcntl(fd, F_SETLKW, &lock) < 0) {
        printf("Tell parent, unlock error!\n");
    }
    printf("Child unlock\n");
    sleep(1);
}

void TELL_CHILD(int fd) {
    struct flock lock;
    lock.l_type = F_UNLCK;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_whence = SEEK_SET;
    if (fcntl(fd, F_SETLKW, &lock) < 0) {
        printf("Tell child, unlock error!\n");
    }
    printf("Parent unlock\n");
    sleep(1);
}

int main() {
    int fd = TELL_WAIT();
    pid_t pid = 0;
    if ((pid = fork()) < 0) {
        printf("Fork error!\n");
        return -1;
    } else if (pid == 0) {
        // Child
        while (1) {
            WAIT_PARENT(fd);
            printf("child process.\n");
            sleep(1);
            TELL_PARENT(fd);
        } 
        exit(0);
    }

    // Parent
    while (1) {
        WAIT_CHILD(fd);
        printf("parent process.\n");
        sleep(10);
        TELL_CHILD(fd);
    }
    return 0;
}