#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#define MSG_MAX_SIZE 512
#define MAX_EPOLL_EVENTS 128

struct my_msgbuf {
    long mtype;  // message type.
    char mtext[MSG_MAX_SIZE];  // message data.
};

struct thread_components {
    int qid;
    int rfd;
    int wfd;
    struct my_msgbuf msg;
    ssize_t msglen;
    pthread_mutex_t mutex;
    pthread_cond_t ready;
};

static void* handler(void * args) {
    struct thread_components* components = (struct thread_components*)(args);
    while (1) {
        ssize_t msglen = 0;
        msglen = msgrcv(components->qid, &components->msg, MSG_MAX_SIZE, 0, MSG_NOERROR);
        if (msglen < 0) {
            printf("Receive message failed!\n");
            return NULL;
        }
        components->msglen = msglen;
        pthread_mutex_lock(&components->mutex);
        if (write(components->wfd, "1", 1) < 0) {
            printf("Write to socket fd failed!\n");
            return NULL;
        }
        pthread_cond_wait(&components->ready, &components->mutex);
        pthread_mutex_unlock(&components->mutex);
    }
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: %s <key_base> <count>\n", argv[0]);
        return -1;
    }

    int epfd = epoll_create1(0);
    if (epfd < 0) {
        printf("Create epoll failed!\n");
        return -1;
    }

    int key_base = atoi(argv[1]);
    int count = atoi(argv[2]);
    for (int i = 0; i < count; ++i) {
        // Attach message queue.
        int qid = msgget(key_base + i, IPC_CREAT);
        if (qid < 0) {
            printf("Attach to message queue failed!\n");
            return -1;
        }

        // Create UNIX domain socket.
        int fd[2];
        if (socketpair(AF_UNIX, SOCK_DGRAM, 0, fd) < 0) {
            printf("Create unix domain socket failed!\n");
            return -1;
        }

        struct thread_components* components =
                (struct thread_components*)malloc(sizeof(struct thread_components));
        components->rfd = fd[0];
        components->wfd = fd[1];
        components->qid = qid;
        pthread_cond_init(&components->ready, NULL);
        pthread_mutex_init(&components->mutex, NULL);

        // Add notification in epoll.
        struct epoll_event ev;
        ev.data.ptr = components;
        ev.events = EPOLLIN;
        if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd[0], &ev) < 0) {
            printf("Add fd to epoll failed!\n");
            return -1;
        }

        // Start thread to monitor msg queue.
        pthread_t tid;
        if (pthread_create(&tid, NULL, handler, components) < 0) {
            printf("Create thread failed!\n");
            return -1;
        }
    }

    struct epoll_event events[MAX_EPOLL_EVENTS];
    while (1) {
        int nfds = epoll_wait(epfd, events, MAX_EPOLL_EVENTS, -1);
        if (nfds < 0) {
            printf("Epoll wait failed!\n");
            return -1;
        }

        for (int i = 0; i < nfds; ++i) {
            struct thread_components* components = (struct thread_components*)events[i].data.ptr;
            // Read from fd.
            char buffer;
            if (read(components->rfd, &buffer, 1) < 0) {
                printf("Read failed!\n");
                return -1;
            }

            printf("Get message from queue[%d]: %s\n", components->qid, components->msg.mtext);
            pthread_mutex_lock(&components->mutex);
            pthread_cond_signal(&components->ready);
            pthread_mutex_unlock(&components->mutex);
        }
    }

    return 0;
}