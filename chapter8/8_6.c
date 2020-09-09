#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {

    pid_t pid;
    if ((pid = fork()) < 0) {
        printf("Fork error!\n");
    } else if (pid == 0) {
        // child 
        exit(7);
    } else {
        // parent
        sleep(2);
        if (system("ps -A -o stat,pid,ppid,cmd | grep 8_6") < 0) {
            printf("Exec failed!\n");
        }
        sleep(20);
    }
    return 0;
}