#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>

#define MSG_MAX_SIZE 512

struct my_msgbuf {
    long mtype;  // message type.
    char mtext[MSG_MAX_SIZE];  // message data.
};

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: %s <key> <message>\n", argv[0]);
        return -1;
    }


    // Get msg queue.
    key_t key = atoi(argv[1]);
    int qid = msgget(key, IPC_CREAT);
    if (qid < 0) {
        printf("Get message queue failed, key:%d\n", key);
        return -1;
    }

    // Send msg to msg queue.
    struct my_msgbuf msg;
    msg.mtype = 1;
    strncpy(msg.mtext, argv[2], MSG_MAX_SIZE - 1);
    int n = msgsnd(qid, &msg, sizeof(msg.mtext), 0);
    if (n < 0) {
        printf("Send msg failed!\n");
        return -1;
    }
    printf("Send msg[%s] success.\n", msg.mtext);
    return 0;
}