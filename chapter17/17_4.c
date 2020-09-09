#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WHITE " \t\n"

int print_args(int argc, char* argv[]) {
    for (int i = 0; i < argc; ++i) {
        printf("arg[%d] %s\n", i, argv[i]);
    }
    return 0;
}

int buf_args(char* buf, int (*optfunc)(int, char**)) {
    int argc = 0;
    char** argv = malloc(sizeof(char*) + sizeof(char*));
    int current_space = sizeof(char*);

    if (strtok(buf, WHITE) == NULL) {
        free(argv);
        return -1;
    }
    argv[argc] = buf;

    char* ptr = NULL;
    while ((ptr = strtok(NULL, WHITE)) != NULL) {
        argc++;
        // Allocate space if needed.
        // Always remain a char* space for terminate NULL.
        int need_space = (argc + 1) * sizeof(char*) + sizeof(char*);
        if (need_space > current_space) {
            current_space <<= 1;
            argv = (char**)realloc(argv, current_space);
        }
        argv[argc] = ptr;
    }
    argv[++argc] = NULL;
    int ret = ((*optfunc)(argc, argv));
    free(argv);
    return ret;
}

int main() {
    char buffer[4096];
    sprintf(buffer, "./test a b c d e");
    buf_args(buffer, print_args);
    return 0;
}