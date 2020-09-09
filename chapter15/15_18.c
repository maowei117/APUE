#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <sys/types.h>
#include <unistd.h>

const char* sem_name_c = "semaphore_c";
const char* sem_name_p = "semaphore_p";

void TELL_WAIT(sem_t** c, sem_t** p) {
    *c = sem_open(sem_name_c, O_CREAT);
    *p = sem_open(sem_name_p, O_CREAT);
}

void TELL_PARENT(sem_t* sem) {
    sem_post(sem);
}

void WAIT_PARENT(sem_t* sem) {
    sem_wait(sem);
}

void TELL_CHILD(sem_t* sem) {
    sem_post(sem);
}

void WAIT_CHILD(sem_t* sem) {
    sem_wait(sem);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const key_t g_key = 0x12345678;
const size_t g_size = 4096;
#define NLOOPS 1000
#define SIZE sizeof(long)

int main() {
    // Create shm.
    int shmid = 0;
    if ((shmid = shmget(g_key, g_size, IPC_CREAT)) < 0) {
        printf("Create shm failed!\n");
        return -1;
    }

    // Attach shm.
    void* addr = shmat(shmid, 0, 0);
    if (addr == (void*)-1) {
        printf("Attach shm failed!\n");
        return -1;
    }
    sem_t* sem_c;
    sem_t* sem_p;
    TELL_WAIT(&sem_c, &sem_p);

    pid_t pid;
    if ((pid = fork()) < 0) {
        printf("Fork failed!\n");
        return -1;
    } else if (pid > 0) {
        // Parent.
        long* value = (long*)(addr);
        for (int i = 0; i < NLOOPS; i += 2) {
            printf("In parent, value:%ld\n", *value);
            *value += 1;
            TELL_CHILD(sem_c);
            WAIT_CHILD(sem_p);
        }
    } else {
        // Child
        long* value = (long*)(addr);
        for (int i = 1; i < NLOOPS + 1; i += 2) {
            WAIT_PARENT(sem_c);
            printf("In child, value:%ld\n", *value);
            *value += 1;
            TELL_PARENT(sem_p);
        }
    }

    return 0;
}