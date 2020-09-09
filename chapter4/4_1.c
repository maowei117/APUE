#include <stdio.h>
#include <sys/stat.h>

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

int main(int argc, char* argv[]) {
    struct stat stat_result;
    struct stat lstat_result;

    for (int i = 1; i < argc; ++i) {
        const char* file = argv[i];
        if (stat(file, &stat_result) != 0) {
            printf("stat error for %s!\n", file);
            return -1;
        }
        if (lstat(file, &lstat_result) != 0) {
            printf("lstat error for %s!\n", file);
            return -1;
        } 
        printf("%s\t", file);
        printf("stat mode:%s\t", mode2typename(stat_result.st_mode));
        printf("lstat mode:%s\n", mode2typename(lstat_result.st_mode));
    }

}