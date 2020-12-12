#include <stdio.h>
#include <time.h>

static const int k_timeBufferSize = 4096;

int main(){
    time_t timestamp = time(NULL);
    if (timestamp < 0) {
        printf("Get current timestamp failed!\n");
        return -1;
    }

    struct tm* gm_time = localtime(&timestamp);

    char buffer[k_timeBufferSize];
    strftime(buffer, k_timeBufferSize, "%Y-%m-%d %A %H:%M:%S %Z", gm_time);
    printf("%s\n", buffer);
    return 0;
}