#include <signal.h>
#include <stdio.h>
#include <string.h>

int sig2str(int signum, char* str) {
    char* s = strsignal(signum);
    if (s == NULL) {
        str = "Invalid signum!\n";
        return -1;
    }
    strcpy(str, s);
    return 0;
}

void test_signal(int signum) {
    char buffer[4096];
    sig2str(signum, buffer);
    printf("Signal:%d String:%s\n", signum, buffer);
}

int main() {
    int test_cases[] = {SIGINT, SIGALRM, SIGABRT};
    int count = sizeof(test_cases) / sizeof(test_cases[0]);
    for (int i = 0; i < count; ++i) {
        test_signal(test_cases[i]);
    }
    return 0;
}