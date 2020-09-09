#include <stdio.h>
#include <time.h>
#include <unistd.h>

int main() {
    char time_buffer[128] = {0};
    while (1) {
        sleep(60);
        time_t timestamp = time(NULL);
        struct tm* local_time = localtime(&timestamp);
        strftime(time_buffer, 128, "%Y-%m-%d %H:%M:%S", local_time);
        printf("%s\n", time_buffer);
    }
    return 0;
}