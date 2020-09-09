#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

pthread_mutex_t lock1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock2 = PTHREAD_MUTEX_INITIALIZER;

void prepare(void) {
    printf("Preparing locks.\n");
    pthread_mutex_lock(&lock1);
    pthread_mutex_lock(&lock2);
}

void child(void) {
    printf("Child unlocking locks.\n");
    pthread_mutex_unlock(&lock1);
    pthread_mutex_unlock(&lock2);
}

void parent(void) {
    printf("Parent unlocking locks.\n");
    pthread_mutex_unlock(&lock1);
    pthread_mutex_unlock(&lock2);
}

static void* thread_func(void* arg) {
    printf("Thread started...\n");
    pause();
    return 0;
}

int main() {
    int err;
    pid_t pid;
    pthread_t tid;
    if ((err = pthread_atfork(prepare, parent, child)) != 0) {
        printf("Can't install handler!\n");
        return -1;
    }
    if ((err = pthread_create(&tid, NULL, thread_func, 0)) != 0) {
        printf("Can't create thread!\n");
        return -1;
    }

    printf("Parent about to fork.\n");
    if ((pid = fork()) < 0) {
        printf("Fork failed!\n");
    } else if (pid == 0) {
        printf("Child returned from fork.\n");
    } else {
        printf("Parent returned from fork.\n");
    }
    return 0;
}