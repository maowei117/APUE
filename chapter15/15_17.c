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

    int value = 0;
    if (pwrite(fd, &value, sizeof(value), 0) != sizeof(value)) { return -1; }
    sync();
    return fd;
}

void WAIT_PARENT(int fd) {
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_whence = SEEK_SET;
    if (fcntl(fd, F_SETLKW, &lock) < 0) {
        printf("Wait parent, lock error!\n");
    }
}

void WAIT_CHILD(int fd) {
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_whence = SEEK_SET;
    if (fcntl(fd, F_SETLKW, &lock) < 0) {
        printf("Wait child, lock error!\n");
    }
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
    sleep(1);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define NLOOPS 10
#define SIZE sizeof(int)

int main() {
    int fd = TELL_WAIT();
    pid_t pid = 0;
    if ((pid = fork()) < 0) {
        printf("Fork error!\n");
        return -1;
    } else if (pid > 0) {
        // Parent.
        int value = 0;
        for (int i = 0; i < NLOOPS; i += 2) {
            WAIT_CHILD(fd);
            if (pread(fd, &value, sizeof(value), 0) != sizeof(value)) { return -1; }
            printf("In parent, value:%d\n", value);
            value += 1;
            if (pwrite(fd, &value, sizeof(value), 0) != sizeof(value)) { return -1; }
            printf("write %d\n", value);
            sync();
            TELL_CHILD(fd);
        }
    } else {
        // Child.
        int value = 0;
        for (int i = 1; i < NLOOPS + 1; i += 2) {
            WAIT_PARENT(fd);
            if (pread(fd, &value, sizeof(value), 0) != sizeof(value)) { return -1; }
            printf("In child, value:%d\n", value);
            value += 1;
            if (pwrite(fd, &value, sizeof(value), 0) != sizeof(value)) { return -1; }
            sync();
            TELL_PARENT(fd);
        }
    }
    return 0;
}