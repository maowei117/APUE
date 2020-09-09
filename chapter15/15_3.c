#include <stdio.h>

int main() {
    FILE* fp = popen("unexist_command", "r");
    if (fp == NULL) {
        printf("Popen failed!\n");
        return -1;
    }

    int ret = pclose(fp);
    printf("ret:%d\n", ret);
    return 0;
}