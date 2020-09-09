#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct barrier {
    int count;
    pthread_mutex_t lock;
    pthread_cond_t cond;
};

int init_barrier(struct barrier* barrier, int count) {
    barrier->count = count;
    pthread_mutex_init(&barrier->lock, NULL);
    pthread_cond_init(&barrier->cond, NULL);
    return 0;
}

void barrier_wait(struct barrier* barrier) {
    pthread_mutex_lock(&barrier->lock);
    barrier->count--;
    printf("count:%d\n", barrier->count);
    if (barrier->count > 0) {
        pthread_cond_wait(&barrier->cond, &barrier->lock);
    } else {
        pthread_cond_broadcast(&barrier->cond);
    }
    pthread_mutex_unlock(&barrier->lock);
}

void destroy_barrier(struct barrier* barrier) {
    if (barrier == NULL) { return; }
    free(barrier);
    barrier = NULL;
}

static void* thread_func(void* arg) {
    struct barrier* barrier = (struct barrier*)(arg);
    int rand = random() % 10;
    printf("Thread[%ld] sleep for %d seconds\n", pthread_self(), rand);
    sleep(rand);
    barrier_wait(barrier);
    printf("Thread[%ld] wake up at %ld\n", pthread_self(), time(NULL));
    return NULL;
}

int main() {
    const int thread_num = 10;

    struct barrier* barrier = (struct barrier*)(malloc(sizeof(struct barrier)));
    if (barrier == NULL) {
        printf("Allocate memory for barrier failed!\n");
        return -1;
    }
    if (init_barrier(barrier, thread_num) != 0) {
        printf("Initialize barrier failed!\n");
        return -1;
    } 

    pthread_t threads[thread_num];
    for (int i = 0; i < thread_num; ++i) {
        if (pthread_create(&threads[i], NULL, thread_func, barrier) != 0) {
            printf("Create thread[%d] failed!\n", i);
        }
    }

    for (int i = 0; i < thread_num; ++i) {
        pthread_join(threads[i], NULL);
    }
    destroy_barrier(barrier);
    return 0;
}
