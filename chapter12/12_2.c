#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

static pthread_mutex_t lock;
static pthread_once_t init_flag = PTHREAD_ONCE_INIT;

static void init_thread(void) {
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&lock, &attr);
    pthread_mutexattr_destroy(&attr);
}

int putenv_r(char* string) {
    pthread_once(&init_flag, init_thread);
    pthread_mutex_lock(&lock);
    int ret = putenv(string);
    pthread_mutex_unlock(&lock);
    return ret;
}

int main() {
    putenv_r("HELLO=WORLD");
    char* env = getenv("HELLO");
    printf("results:%s\n", env);
    return 0;
}