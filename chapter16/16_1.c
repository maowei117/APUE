#include <stdio.h>

int main() {
    int a = 0x12345678;
    for (int i = 0; i < sizeof(a); ++i) {
        char* addr = (char*)&a + i;
        printf("addr:%p value:%x\t", addr, *addr);
    }
    printf("\n");

    char* addr = (char*)&a;
    if (*addr == 0x12) {
        printf("Big endian.\n");
    } else if (*addr == 0x78) {
        printf("Little endian.\n");
    } else {
        printf("unknow.\n");
    }
    return 0;
}