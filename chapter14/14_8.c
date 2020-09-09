#include <aio.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>
#include <errno.h>

// Here buf size must be 1.
#define k_bufSize  1
#define k_blockSize 4096

enum aio_op {
    aio_op_dummy = 0,
    aio_op_reading = 1,
    aio_op_writing = 2,
};

struct aio_buffer {
    enum aio_op op;
    struct aiocb aiocb;
    char data[k_blockSize];
};

int main() {
    struct aio_buffer aio_buffer[k_bufSize];
    const struct aiocb* aiolist[k_bufSize];

    // Open log file.
    int log_fd = open("./14_8.log", O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (log_fd < 0) { printf("Open error!\n"); return -1; }
    char log_buffer[128];

    // Initialize aio buffer.
    for (int i = 0; i < k_bufSize; ++i) {
        aio_buffer[i].op = aio_op_dummy;
        aio_buffer[i].aiocb.aio_buf = aio_buffer[i].data;
        aio_buffer[i].aiocb.aio_sigevent.sigev_notify = SIGEV_NONE;
        memset(aio_buffer[i].data, 0, k_blockSize);
    }

    // Initialize select buffer.
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    
    // Use aio to read data from stdin and write data to stdout.
    int count = 0;
    int finished = 0;
    while (1) {
        for (int i = 0; i < k_bufSize; ++i) {
            switch (aio_buffer[i].op) {
                case aio_op_dummy: {
                    int ret = 0;
                    if ((ret = select(1, &readfds, NULL, NULL, &timeout)) < 0) {
                        printf("Select failed\n");
                        return -1;
                    } else if (ret == 0) {
                        finished = 1;
                        continue;
                    } else {
                        // Stdin has data.
                        if (!finished) {
                            aio_buffer[i].op = aio_op_reading;
                            aio_buffer[i].aiocb.aio_fildes = STDIN_FILENO;
                            aio_buffer[i].aiocb.aio_nbytes = k_blockSize;
                            aiolist[i] = &aio_buffer[i].aiocb;
                            if (aio_read(&aio_buffer[i].aiocb) < 0) {
                                printf("Submit aio request error!\n");
                                return -1;
                            }
                            count++;
                        }
                    }
                    break;
                }
                case aio_op_reading: {
                    int err = 0;
                    if ((err = aio_error(&aio_buffer[i].aiocb)) == EINPROGRESS) {
                        continue;
                    }
                    if (err != 0) {
                        printf("Unexpected error!\n");
                        return -1;
                    }
                    int n = 0;
                    if ((n = aio_return(&aio_buffer[i].aiocb)) < 0) {
                        printf("Aio return failed!\n");
                        return -1;
                    }
                    if (n != k_blockSize) { finished = 1; }

                    aio_buffer[i].op = aio_op_writing;
                    aio_buffer[i].aiocb.aio_fildes = STDOUT_FILENO;
                    aio_buffer[i].aiocb.aio_nbytes = k_blockSize;
                    if (aio_write(&aio_buffer[i].aiocb) < 0) {
                        printf("Aio write failed!\n");
                        return -1;
                    }
                    break;
                }
                case aio_op_writing: {
                    int err = 0;
                    if ((err = aio_error(&aio_buffer[i].aiocb)) == EINPROGRESS) {
                        continue;
                    }
                    if (err != 0) {
                        printf("Unexpected error!\n");
                        return -1;
                    }
                    int n = 0;
                    if ((n = aio_return(&aio_buffer[i].aiocb)) < 0) {
                        printf("Aio return failed!\n");
                        return -1;
                    }
                    aio_buffer[i].op = aio_op_dummy;
                    aiolist[i] = NULL;
                    count--;
                    break;
                }
                default:
                    break;
            }
            sleep(1);
        }

        int n = snprintf(log_buffer, 128, "finished loop, count:%d, finished:%d\n", count, finished);
        if (write(log_fd, log_buffer, n + 1) < 0) { return -1; }

        if (count == 0) {
            if (finished) { break; }
        }

        // Wait till at least 1 aio complete.
        if (aio_suspend(aiolist, k_bufSize, NULL) < 0) {
            printf("Aio suspend error!\n");
            return -1;
        }
    }
}