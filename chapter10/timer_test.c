#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>


struct priv_obj {
    timer_t** timer_array;
    int timer_count;
    const char* message;
};

static void handler(int signum, siginfo_t *si, void* uc) {
    struct priv_obj* priv = si->si_value.sival_ptr;

    const char* message = priv->message;
    printf("Handle alarm, message:%s\n", message);

    int timer_count = priv->timer_count;
    timer_t** timer_array = (timer_t**)priv->timer_array;
    struct itimerspec cur_value;
    for (int i = 0; i < timer_count; ++i) {
        timer_t id = *(timer_array[i]);
        if (timer_gettime(id, &cur_value) != 0) {
            printf("Get value failed!\n");
        }
        printf("Left time for timer %d: %ld sec %ld nsec.\n",
                i + 1, cur_value.it_value.tv_sec, cur_value.it_value.tv_nsec);
    }

    printf("Handler timer, message:%s\n", message);
}

int main() {
    // Register signal handler for SIGALRM.
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handler;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGALRM, &sa, NULL) != 0) {
        printf("Register signal handler failed!\n");
        return -1;
    }

    timer_t* timer_array[2];
    int timer_count = sizeof(timer_array) /  sizeof(timer_array[0]);

    // Create timer 1.
    timer_t timer_id_1;
    timer_array[0] = &timer_id_1;
    struct sigevent sev1;
    sev1.sigev_notify = SIGEV_SIGNAL;
    sev1.sigev_signo = SIGALRM;
    struct priv_obj priv1;
    priv1.timer_array = timer_array;
    priv1.timer_count = timer_count;
    priv1.message = "timer 1";
    sev1.sigev_value.sival_ptr = &priv1;
    if (timer_create(CLOCK_REALTIME, &sev1, &timer_id_1) != 0) {
        printf("Create timer 1 failed!\n");
        return -1;
    }

    // Create timer 2.
    timer_t timer_id_2;
    timer_array[1] = &timer_id_2;
    struct sigevent sev2;
    sev2.sigev_notify = SIGEV_SIGNAL;
    sev2.sigev_signo = SIGALRM;

    struct priv_obj priv2;
    priv2.timer_array = timer_array;
    priv2.timer_count = timer_count;
    priv2.message = "timer 2";
    sev2.sigev_value.sival_ptr = &priv2;
    if (timer_create(CLOCK_REALTIME, &sev2, &timer_id_2) != 0) {
        printf("Create timer 1 failed!\n");
        return -1;
    }

    // Start timer1.
    struct itimerspec settime1;
    settime1.it_interval.tv_sec = 2;
    settime1.it_interval.tv_nsec = 0;
    settime1.it_value.tv_sec = 2;
    settime1.it_value.tv_nsec = 0;
    if (timer_settime(timer_id_1, 0, &settime1, NULL) != 0) {
        printf("Set timer 1 failed!\n");
        return -1;
    }

    // Sleep a random time before start timer2.
    srandom(time(NULL));
    int usleep_gap = random() % 1000000;
    usleep(usleep_gap);

    // Start timer2
    struct itimerspec settime2;
    settime2.it_interval.tv_sec = 1;
    settime2.it_interval.tv_nsec = 0;
    settime2.it_value.tv_sec = 1;
    settime2.it_value.tv_nsec = 0;
    if (timer_settime(timer_id_2, 0, &settime2, NULL) != 0) {
        printf("Set timer 2 failed!\n");
        return -1;
    }

    // Loop to check left time.
    int counter = 0;
    while (counter < 20) {
        pause();
        counter++;
    } 

    // Delete timer.
    if (timer_delete(timer_id_1) != 0) {
        printf("Delete timer 1 failed!\n");
        return -1;
    }

    if (timer_delete(timer_id_2) != 0) {
        printf("Delete timer 2 failed!\n");
        return -1;
    }
    return 0;
}