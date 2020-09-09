#include <stdio.h>
#include <sys/select.h>

void sleep_us(unsigned int usecs) {
    struct timeval val;
    val.tv_sec = usecs / 1000000;
    val.tv_usec = usecs % 1000000;
    select(0, NULL, NULL, NULL, &val);
}

int main() {
    printf("Before sleep\n");
    sleep_us(1000000);
    printf("After sleep\n");
    return 0;
}