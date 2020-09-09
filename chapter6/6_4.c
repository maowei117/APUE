#include <limits.h>
#include <stdio.h>
#include <time.h>

static const int k_timeBufferSize = 4096;

int main(){
    time_t t;
    if (time(&t) < 0) {
        printf("Get time failed!\n");
    }
    printf("time_t size[%zu], current timestamp[%ld]\n", sizeof(t), (long)t);

    struct tm* tm;
    tm = gmtime(&t);

    char time_buffer[k_timeBufferSize];
    strftime(time_buffer, k_timeBufferSize, "[%Z] %Y-%m-%d %H:%M:%S", tm);
    printf("Gmt time[%s]\n", time_buffer);

    t = INT_MAX;
    tm = gmtime(&t);
    strftime(time_buffer, k_timeBufferSize, "[%Z] %Y-%m-%d %H:%M:%S", tm);
    printf("Largest gmt time[%s]\n", time_buffer);
    return 0;
}