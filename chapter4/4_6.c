#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>

static const long k_defaultBlockSize = 4096;

long get_block_size() {
    long size = sysconf(_SC_PAGESIZE);
    if (size < 0) {
        return k_defaultBlockSize;
    }
    return size;
}

bool is_hole(char* buffer, ssize_t size) {
    for (ssize_t i = 0; i < size; ++i) {
        if (buffer[i] != 0) {
            return false;
        }
    }
    return true;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: %s <src> <dst>\n", argv[0]);
        return -1;
    }

    const char* src = argv[1];
    const char* dst = argv[2];

    int src_fd = open(src, O_RDONLY);
    if (src_fd == -1) {
        printf("Open src file[%s] failed!\n", src);
        return -1;
    }
    int dst_fd = open(dst, O_WRONLY | O_CREAT, 0644);
    if (dst_fd == -1) {
        printf("Open dst file[%s] failed!\n", dst);
        return -1;
    }

    long block_size = get_block_size();
    char* buffer = (char*)malloc(block_size);
    ssize_t read_size = 0;
    while ((read_size = read(src_fd, buffer, block_size)) > 0) {
        if (is_hole(buffer, read_size)) {
            lseek(dst_fd, block_size, SEEK_CUR);
            continue;
        }
        if (write(dst_fd, buffer, read_size) < 0) { return -1; }
    }

    close(src_fd);
    close(dst_fd);
    return 0;
}