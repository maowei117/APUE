#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>

#define COPYINCR (1 << 30)  // 1GB

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: %s <srcfile> <dstfile>\n", argv[0]);
        return -1;
    }

    // Open files
    int src_fd = open(argv[1], O_RDONLY);
    if (src_fd < 0) {
        printf("Open src file failed!\n");
        return -1;
    }

    int dst_fd = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (dst_fd < 0) {
        printf("Open dst file failed!\n");
        return -1;
    }

    // Get src file stat.
    struct stat sbuf;
    if (fstat(src_fd, &sbuf) < 0) {
        printf("Get src file stat failed!\n");
        return -1;
    }

    if (ftruncate(dst_fd, sbuf.st_size) < 0) {
        printf("Truncate dst file size failed!\n");
        return -1;
    }

    // Do copy.
    size_t copy_size = 0;
    off_t offset = 0;
    while (offset < sbuf.st_size) {
        if (sbuf.st_size - offset > COPYINCR) {
            copy_size = COPYINCR;
        } else {
            copy_size = sbuf.st_size - offset;
        }

        // Mmap.
        void* src = NULL;
        if ((src = mmap(0, copy_size, PROT_READ, MAP_SHARED, src_fd, offset)) == MAP_FAILED) {
            printf("src file mmap failed!\n");
            return -1;
        }

        void* dst = NULL;
        if ((dst = mmap(0, copy_size, PROT_READ | PROT_WRITE, MAP_SHARED, dst_fd, offset)) == MAP_FAILED) {
            printf("dst file mmap failed!\n");
            return -1;
        }

        // Here close fd won't disable mmap.
        // close(src_fd);
        // close(dst_fd);

        memcpy(dst, src, copy_size);
        munmap(src, copy_size);
        munmap(dst, copy_size);
        offset += copy_size;
    } 

    return 0;
}