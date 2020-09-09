#include <unistd.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <path_name>\n", argv[0]);
        return -1;
    }
    const char* path_name = argv[1];
    if (unlink(path_name) == 0) {
        printf("unlink ok\n");
    }
    sleep(20);
    printf("done\n");
    return 0;
}