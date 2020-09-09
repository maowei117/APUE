#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main() {
    const char* root = "/";
    DIR* dir = opendir(root);
    int dir_fd = dirfd(dir);

    int flag = fcntl(dir_fd, F_GETFD);
    printf("opendir(), close_exec_flag:%d, CLOEXEC:%d\n", flag, FD_CLOEXEC);

    int fd = open(root, O_RDONLY);
    int flag2 = fcntl(fd, F_GETFD);
    printf("open(), close_exec_flag:%d, CLOEXEC:%d\n", flag2, FD_CLOEXEC);
    return 0;
}