#include <stdio.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <unistd.h>

const char* mode2typename(mode_t mode) {
    if (S_ISREG(mode)) {
        return "regular";
    } else if (S_ISDIR(mode)) {
        return "directory";
    } else if (S_ISCHR(mode)) {
        return "character special";
    } else if (S_ISBLK(mode)) {
        return "block special";
    } else if (S_ISFIFO(mode)) {
        return "fifo";
    } else if (S_ISLNK(mode)) {
        return "symbolic link";
    } else if (S_ISSOCK(mode)) {
        return "socket";
    }
    return "unknown mode";
}

int main() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        printf("Create socket failed!\n");
        return -1;
    }

    struct stat statbuf;
    if (fstat(fd, &statbuf) < 0) {
        printf("Get stat failed!\n");
        return -1;
    }

    printf("inode:%lu\n", statbuf.st_ino);
    printf("link num:%lu\n", statbuf.st_nlink);
    printf("device id:%lu major:%u minor:%u\n", statbuf.st_dev, major(statbuf.st_dev), minor(statbuf.st_dev));
    printf("mode:%s\n", mode2typename(statbuf.st_mode));
    printf("uid:%u gid:%u\n", statbuf.st_uid, statbuf.st_gid);
    printf("size:%ld\n", statbuf.st_size);
    printf("block size:%ld\n", statbuf.st_blksize);
    printf("blocks:%ld\n", statbuf.st_blocks);
    printf("atime:%ld\n", statbuf.st_atim.tv_sec);
    printf("mtime:%ld\n", statbuf.st_mtim.tv_sec);
    printf("ctime:%ld\n", statbuf.st_ctim.tv_sec);
    return 0;
}