#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

struct foo {
    int a;
    int b;
};

void* thread_func(void* arg) {
    struct foo* foo = (struct foo*)(arg);
    printf("In thread_func: foo.a=%d foo.b=%d\n", foo->a, foo->b);
    foo->a = 3;
    foo->b = 4;
    return 0;
}

int main() {
    struct foo* foo = malloc(sizeof(struct foo));
    if (foo == NULL) {
        printf("malloc failed!\n");
        return -1;
    }
    foo->a = 0;
    foo->b = 1;

    pthread_t t1;
    if (pthread_create(&t1, NULL, thread_func, foo) != 0) {
        printf("Create thread failed!\n");
        return -1;
    }
    if (pthread_join(t1, NULL) != 0) {
        printf("Wait pthread failed!\n");
        return -1;
    }
    printf("After join: foo.a=%d foo.b=%d\n", foo->a, foo->b);
    return 0;
}