#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <sys/resource.h>
#include <unistd.h>

long open_max() {
    long open_max = -1;
    if ((open_max = sysconf(_SC_OPEN_MAX)) < 0 || open_max == LONG_MAX) {
        struct rlimit limit;
        if (getrlimit(RLIMIT_NOFILE, &limit) < 0) {
            printf("Get resouce limit failed!\n");
            return -1;
        }
        if (limit.rlim_max == RLIM_INFINITY) {
            open_max = 255;
        } else {
            open_max = limit.rlim_max;
        }
    }
    return open_max;
}

int main() {
    printf("Open max : %ld\n", open_max());
    return 0;
}