#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int g_var = 6;

int main(void) {
    int var;
    pid_t pid;

    var = 88;
    printf("before vfork\n");
    if ((pid = vfork()) < 0) {
        printf("vfork error!\n");
    } else if (pid == 0) {
        // child
        g_var++;
        var++;
        fclose(stdout);
        exit(0);
    } else {
        // parent
        int ret = printf("pid = %ld, g_var = %d, var = %d\n", (long)(pid), g_var, var);
        exit(ret);
    }
}