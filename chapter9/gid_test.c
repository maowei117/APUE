#include <stdio.h>
#include <unistd.h>

int main() {
    pid_t parent_gid = getpgid(0);
    pid_t parent_pid = getpid();
    pid_t child_gid = -1;
    pid_t child_pid = -1;
    printf("Before fork(): pid:%d gid:%d\n", parent_pid, parent_gid);

    pid_t pid = fork();
    if (pid == 0) {
        // child
        child_gid = getpgid(0);
        child_pid = getpid();
        printf("In child: pid:%d gid:%d\n", child_pid, child_gid);
    } else if (pid > 0) {
        child_pid = pid;
        // Sleep to wait child run first.
        sleep(2);

        // parent
        parent_gid = getpgid(0);
        parent_pid = getpid();
        printf("In parent: parent pid:%d gid:%d\n", parent_pid, parent_gid);

        // Make child be a new group leader.
        if (setpgid(child_pid, 0) != 0) {
            printf("Set process gid failed!\n");
            return -1;
        }

        parent_gid = getpgid(0);
        child_gid = getpgid(child_pid);
        printf("After setpgid, parent pid:%d gid:%d \t child pid:%d gid:%d\n",
               parent_pid, parent_gid, child_pid, child_gid);
    } else {
        printf("Fork error!\n");
    }
    return 0;
}
