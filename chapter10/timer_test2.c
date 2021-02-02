#include <signal.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

static void handler(int signum) {
    // Get current time.
    char timestr[128];
    time_t timestamp = time(NULL);
    struct tm* local_time = localtime(&timestamp);
    strftime(timestr, 128, "%Y-%m-%d %H:%M:%S", local_time);
    printf("current time:%s\n", timestr);
}

int main() {
    // Register alarm handler.
    struct sigaction act;
    act.sa_handler = handler;
    act.sa_flags = 0;
    if (sigaction(SIGALRM, &act, NULL) != 0) {
        printf("Register signal handler failed!\n");
        perror("");
        return -1;
    }

    // Use alarm, set 1s interval.
    printf("alarm:1s\n");
    alarm(1);
    int counter = 0;
    while (counter < 10) {
        alarm(1);
        pause();
        counter++;
    }

    // Edit alarm interval to 5s.
    printf("alarm:5s\n");
    alarm(5);
    counter = 0;
    while (counter < 10) {
        alarm(5);
        pause();
        counter++;
    }

    // Delete alarm.
    alarm(0);

    // Use setitimer, set 1s interval.
    printf("setitimer:1s\n");
    struct itimerval interval;
    interval.it_value.tv_sec = 1;
    interval.it_value.tv_usec = 0;
    interval.it_interval.tv_sec = 1;
    interval.it_interval.tv_usec = 0;
    setitimer(ITIMER_REAL, &interval, NULL);
    counter = 0;
    while (counter < 10) {
        pause();
        counter++;
    }

    // Edit timer interval to 3s.
    printf("setitimer:3s\n");
    interval.it_value.tv_sec = 3;
    interval.it_value.tv_usec = 0;
    interval.it_interval.tv_sec = 3;
    interval.it_interval.tv_usec = 0;
    setitimer(ITIMER_REAL, &interval, NULL);
    counter = 0;
    while (counter < 10) {
        pause();
        counter++;
    }

    return 0;
}