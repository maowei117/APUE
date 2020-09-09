#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct job {
    struct job* prev;
    struct job* next;
    pthread_t job_id;
    pthread_mutex_t lock;
};

struct queue {
    struct job* head;
    struct job* tail;
    pthread_rwlock_t lock;
    pthread_t* lazy_thread;
    pthread_t* hardworking_thread;
};

int queue_init(struct queue* q) {
    q->head = NULL;
    q->tail = NULL;
    return pthread_rwlock_init(&q->lock, NULL);
}

// Insert job to queue head.
int job_insert(struct queue* q, struct job* job) {
    if (job == NULL) { return -1; }
    pthread_rwlock_wrlock(&q->lock);
    struct job* old_head = q->head;
    if (old_head != NULL) {
        old_head->prev = job;
    } else {
        q->tail = job;
    }
    job->prev = NULL;
    job->next = old_head;
    q->head = job;
    pthread_rwlock_unlock(&q->lock);
    return 0;
}

// Append job to queue tail.
int job_append(struct queue* q, struct job* job) {
    if (job == NULL) { return -1; }
    pthread_rwlock_wrlock(&q->lock);
    struct job* old_tail = q->tail;
    if (old_tail != NULL) {
        old_tail->next = job;
    } else {
        q->head = job;
    }
    job->prev = old_tail;
    job->next = NULL;
    q->tail = job;
    pthread_rwlock_unlock(&q->lock);
    return 0;
}

// Remove specific job from queue.
int job_remove(struct queue* q, struct job* job) {
    if (job == NULL) { return -1; }
    pthread_rwlock_wrlock(&q->lock);
    struct job* cur_job = NULL;
    for (cur_job = q->head; cur_job != job; cur_job = cur_job->next) {
        if (cur_job == NULL) { 
            pthread_rwlock_unlock(&q->lock);
            return -1;
        }
    }
    struct job* prev = cur_job->prev;
    struct job* next = cur_job->next;
    if (q->head == cur_job) {
        q->head = next;
    }
    if (q->tail == cur_job) {
        q->tail = prev;
    }
    if (prev != NULL) {
        prev->next = next;
    }
    if (next != NULL) {
        next->prev = prev;
    }
    cur_job->prev = NULL;
    cur_job->next = NULL;
    pthread_rwlock_unlock(&q->lock);
    return 0;
}

int job_change(struct queue* q, struct job* job, pthread_t job_id) {
    if (q == NULL || job == NULL) { return -1; }
    pthread_rwlock_rdlock(&q->lock);
    pthread_mutex_lock(&job->lock);
    struct job* cur_job = q->head;
    while (cur_job != NULL) {
        if (cur_job == job) { break; }
        cur_job = cur_job->next;
    }
    if (cur_job != NULL) {
        printf("Change job[%p] from %ld to %ld.\n", cur_job, cur_job->job_id, job_id);
        cur_job->job_id = job_id;
    }
    pthread_mutex_unlock(&job->lock);
    pthread_rwlock_unlock(&q->lock);
    return 0;
}

struct job* job_find(struct queue* q, pthread_t job_id) {
    pthread_rwlock_rdlock(&q->lock);
    struct job* cur_job = q->head;
    while (cur_job != NULL) {
        if (pthread_equal(cur_job->job_id, job_id)) { break; }
        cur_job = cur_job->next;
    }
    pthread_rwlock_unlock(&q->lock);
    return cur_job;
}

void print_jobs(struct queue* q) {
    pthread_rwlock_rdlock(&q->lock);
    printf("NULL-");
    struct job* cur_job = q->head;
    for (; cur_job != NULL; cur_job = cur_job->next) {
        printf("%ld-", cur_job->job_id);
    }
    printf("NULL\n");
    pthread_rwlock_unlock(&q->lock);
}

void job_process(struct job* job) {
    printf("Process job[%p] id[%ld]\n", job, job->job_id);
    free(job);
}

static void* thread_func(void* arg) {
    struct queue* queue = (struct queue*)(arg);
    pthread_t lazy_thread = *(queue->lazy_thread);
    pthread_t hardworking_thread = *(queue->hardworking_thread);
    pthread_t id = pthread_self();
    while (1) {
        struct job* job = job_find(queue, id);
        if (job != NULL) {
            if (pthread_equal(lazy_thread, id)) {
                job_change(queue, job, hardworking_thread);
            } else {
                if (job_remove(queue, job) < 0) {
                    printf("Remove job failed, job[%p]!\n", job);
                    return NULL;
                }
                job_process(job);
            }
            print_jobs(queue);
        }
        sleep(1);
    }
    return 0;
}

int main() {
    // Initialize job queue.
    struct queue* job_queue = (struct queue*)malloc(sizeof(struct queue));
    if (job_queue == NULL) {
        printf("Alloc resource for queue failed!\n");
        return -1;
    }
    if (queue_init(job_queue) != 0) {
        printf("Initialize queue failed!\n");
        return -1;
    }

    // Create worker threads.
    const int thread_num = 3;
    pthread_t threads[thread_num];
    job_queue->lazy_thread = &threads[0];
    job_queue->hardworking_thread = &threads[1];
    for (int i = 0; i < thread_num; ++i) {
        if (pthread_create(&threads[i], NULL, thread_func, job_queue) != 0) {
            printf("Create thread[%d] failed!\n", i);
        }
    }

    // Add jobs to queue.
    const int total = 10;
    srand(time(NULL));
    for (int i = 0; i < total; ++i) {
        struct job* job = (struct job*)malloc(sizeof(struct job));
        if (job == NULL) {
            printf("Initialize job failed!\n");
        }
        int rand = random();
        int tid = rand % thread_num;
        job->job_id = threads[tid];
        if (tid % 2 == 0) {
            job_insert(job_queue, job);
        } else {
            job_append(job_queue, job);
        }
    }
    printf("Add jobs ok:\n");
    print_jobs(job_queue);

    for (int i = 0; i < thread_num; ++i) {
        pthread_join(threads[i], NULL);
    }
    return 0;
}