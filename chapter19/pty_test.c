#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int main() {
    int master_fd = posix_openpt(O_RDWR);
    if (master_fd < 0) {
        printf("Create master fd failed!\n");
        return -1;
    }

    char slave_name[4096];
    if (ptsname_r(master_fd, slave_name, 4096) < 0) {
        printf("Get slave name failed!\n");
        return -1;
    }
    printf("Slave name:%s\n", slave_name);

    struct stat statbuf;
    if (stat(slave_name, &statbuf) < 0) {
        printf("Get stat failed!\n");
        return -1;
    }
    printf("uid:%d gid:%d\n", statbuf.st_uid, statbuf.st_gid);

    if (grantpt(master_fd) < 0) {
        printf("Grant tty failed!\n");
        return -1;
    }

    if (stat(slave_name, &statbuf) < 0) {
        printf("Get stat failed!\n");
        return -1;
    }
    printf("uid:%d gid:%d\n", statbuf.st_uid, statbuf.st_gid);

    return 0;
}