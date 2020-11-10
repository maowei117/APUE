#include <dirent.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

static const int k_maxPathLength = 4096;

struct FileTypeCounter {
    int socket;
    int symbolic_link;
    int regular_file;
    int block_device;
    int directory;
    int character_device;
    int fifo;
    int unknown;
};

bool ftw_absolute(char* start_path);
bool do_ftw_absolute(char* current_path, struct FileTypeCounter* counter);

void add_counter(const struct stat* statbuf, struct FileTypeCounter* counter);
void report_counter(const struct FileTypeCounter* counter);


int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <start_path>\n", argv[0]);
        return -1;
    }
    ftw_absolute(argv[1]);
    return 0;
}

bool ftw_absolute(char* start_path) {
    struct FileTypeCounter counter = { 0, 0, 0, 0, 0, 0, 0, 0, };
    char path_buffer[k_maxPathLength];
    strcpy(path_buffer, start_path);
    do_ftw_absolute(path_buffer, &counter);
    report_counter(&counter);
    return true;
}

bool do_ftw_absolute(char* current_path, struct FileTypeCounter* counter) {
    // Record current file into counter
    struct stat statbuf;
    if (lstat(current_path, &statbuf) != 0) {
        printf("Get stat for file [%s] failed!\n", current_path);
        return false;
    }
    add_counter(&statbuf, counter);

    // Finish condition, if file type is not directory, just return.
    if (!S_ISDIR(statbuf.st_mode)) {
        return true;
    }

    // When current file type is directory, count all files under this directory.
    DIR* dirp = opendir(current_path);
    if (dirp == NULL) {
        printf("Open directory [%s] failed!\n", current_path);
        return false;
    }

    // Record current_path length
    size_t current_path_length = strlen(current_path);

    struct dirent* entry_p;
    while ((entry_p = readdir(dirp)) != NULL) {
        const char* file_name = entry_p->d_name;
        // SKip '.' and '..'
        if (strcmp(".", file_name) == 0 || strcmp("..", file_name) == 0) { continue; }

        // Generate new path
        size_t len = current_path_length;
        current_path[len++] = '/';
        strcpy(current_path + len, file_name);

        do_ftw_absolute(current_path, counter);
    }

    // Recover current_path
    current_path[current_path_length] = '\0';
    return true;
}

void add_counter(const struct stat* statbuf, struct FileTypeCounter* counter) {
    switch (statbuf->st_mode & S_IFMT) {
        case S_IFSOCK:
            counter->socket++;
            break;
        case S_IFLNK:
            counter->symbolic_link++;
            break;
        case S_IFREG:
            counter->regular_file++;
            break;
        case S_IFBLK:
            counter->block_device++;
            break;
        case S_IFDIR:
            counter->directory++;
            break;
        case S_IFCHR:
            counter->character_device++;
            break;
        case S_IFIFO:
            counter->fifo++;
            break;
        default:
            counter->unknown++;
            break;
    }
}

void report_counter(const struct FileTypeCounter* counter) {
    printf("===================Report===================\n");
    printf("socket: %d\n", counter->socket);
    printf("symbolic_link: %d\n", counter->symbolic_link);
    printf("regular_file: %d\n", counter->regular_file);
    printf("block_device: %d\n", counter->block_device);
    printf("directory: %d\n", counter->directory);
    printf("character_device: %d\n", counter->character_device);
    printf("fifo: %d\n", counter->fifo);
    printf("unkonwn:%d\n", counter->unknown);
}
