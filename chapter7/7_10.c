#include <stdio.h>

int main(int argc, char* argv[]) {
    int num = 0;
    int* ptr = &num;
    if (argc > 1) {
        int val;
        val = 5;
        ptr = &val;
    }
    int a = 1;
    printf("val[%d] a[%d]\n", (*ptr + 1), a);
    return 0;
}