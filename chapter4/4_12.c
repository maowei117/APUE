#include <stdio.h>
#include <unistd.h>

static const int k_pathMaxLength = 4096;

int main(int argc, char* argv[]) {
    char path_buffer[k_pathMaxLength];
    // Get current working dir
    if (getcwd(path_buffer, k_pathMaxLength) == NULL) {
        printf("Get current working dir failed!\n");
        return -1;
    }
    printf("current working dir[%s]\n", path_buffer);
    // Change root to ./CMakeFiles
    if (chroot("./CMakeFiles") != 0) {
        printf("Chroot failed!\n");
        return -1;
    }
    // chdir to /4_12.dir
    if (chdir("/4_12.dir") != 0) {
        printf("Chdir to /4_12.dir failed!\n");
        return -1;
    }
    printf("Chdir to /4_12.dir success!\n");
    if (getcwd(path_buffer, k_pathMaxLength) == NULL) {
        printf("Get current working dir failed!\n");
        return -1;
    }
    printf("current working dir[%s]\n", path_buffer);
    // Chdir to ../../
    if (chdir("../../") != 0) {
        printf("Chdir to ../../ failed!\n");
        return -1;
    }
    printf("Chdir to ../../ success!\n");
    if (getcwd(path_buffer, k_pathMaxLength) == NULL) {
        printf("Get current working dir failed!\n");
        return -1;
    }
    printf("current working dir[%s]\n", path_buffer);
    return 0;
}