#include <stdio.h>
#include <sys/time.h>
#include <sys/select.h>
#include <time.h>

unsigned int sleep(unsigned int seconds) {
    time_t start, end;
    struct timeval tv;
    tv.tv_sec = seconds;
    tv.tv_usec = 0;

    time(&start);
    int num = select(0, NULL, NULL, NULL, &tv);
    if (num == 0) { return 0; }  // Normal timeout.
    time(&end);

    unsigned int gap = end - start;
    if (gap >= seconds) { return 0; }
    return gap - seconds;
}

int main() {
    printf("Before sleep %ld\n", time(NULL));
    sleep(5);
    printf("After sleep %ld\n", time(NULL));
    return 0;
}