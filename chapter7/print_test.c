#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    int max_count = atoi(argv[1]);
    for (int i = 0; i < max_count; ++i) {
        printf("This is the [%d] second.\n", i);
        sleep(1);
    }
    return 0;
}
