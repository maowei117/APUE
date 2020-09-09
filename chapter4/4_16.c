#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>


static const char* k_dirName = "test_dir_name";

int main() {
    printf("MAX_PATH = %d\n", PATH_MAX);

    // Loop to make directory.
    int loop_count = 2000;
    for (int i = 0; i < loop_count; ++i) {
        if (mkdir(k_dirName, 0777) != 0) {
            printf("Mkdir failed, loop_count[%d]!\n", i);
            return -1;
        }

        if (chdir(k_dirName) != 0) {
            printf("Chdir failed, loop_count[%d]!\n", i);
            return -1;
        }
    }

    // Test getcwd
    int path_size = PATH_MAX;
    char* path_buffer = (char*)malloc(path_size);
    int retry_max = 5;
    int retry_round = 0;
    while (getcwd(path_buffer, path_size) == NULL) {
        if (retry_round >= retry_max) {
            printf("Retry round >= %d, getcwd failed, current_path_size[%d]", retry_max, path_size);
            return -1;
        }

        printf("Getcwd failed, current buffer size[%d], retry to realloc!\n", path_size);
        path_size <<= 1;
        path_buffer = (char*)realloc(path_buffer, path_size);
    }
    printf("Get cwd success, path_size[%d], cwd[%s]", path_size, path_buffer);
    return 0;
}