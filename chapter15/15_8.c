#include <stdio.h>

int main() {
    const char* cmdstring = "./15_8_2";
    FILE* fp = popen(cmdstring, "r");

    char buffer[128];
    if (fread(buffer, 128, 1, fp) != 1) { return -1; }
    printf("In caller:%s\n", buffer);
    return 0;
}