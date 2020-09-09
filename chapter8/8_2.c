#include <unistd.h>
#include <stdio.h>

void func1() {
    pid_t pid;
    if ((pid = vfork()) < 0) {
        printf("vfork() error!\n");
        return;
    } else if (pid == 0) {
        // child
        return;
    }

    return;
}

void func2(int a) {
    printf("%d\n", a);
    return;
}

int main() {
    func1();
    func2(1);
    return 0;
}