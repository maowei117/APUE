#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

int main() {
    // Loop to create msg queue.
    int loop_count = 5;
    int key_base = 1122334;
    for (int i = 0; i < loop_count; ++i) {
        key_t key = i + key_base;
        int msqid = msgget(key, IPC_CREAT);
        if (msqid < 0) {
            printf("Create msg queue failed!\n");
            return -1;
        }
        printf("Create msg queue success, loop:%d key:%d msqid:%d:%x\n", i, key, msqid, msqid);

        if (msgctl(msqid, IPC_RMID, NULL) != 0) {
            printf("Delete msq queue failed!\n");
            return -1;
        }
    }

    for (int i = 0; i < loop_count; ++i) {
        int msqid = msgget(IPC_PRIVATE, 0);
        if (msqid < 0) {
            printf("Create msg queue failed!\n");
            return -1;
        }
        printf("IPC_PRIVATE create msg queue success, msqid:%d:%x\n", msqid, msqid);
    }

    return 0;
}