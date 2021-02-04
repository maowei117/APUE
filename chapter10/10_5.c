#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

typedef void (*timer_handler) (int signum);
struct timer_node {
    timer_handler handler;
    time_t timestamp;
    int timerid;
};

struct timer_context {
    struct timer_node* nodes;
    int capacity;
    int size;

    struct sigaction act;
    struct itimerval base_timeval;
};

int timer_node_compare(const struct timer_node* lhs, const struct timer_node* rhs) {
    if (lhs->timestamp < rhs->timestamp) {
        return -1;
    } else if (lhs->timestamp == rhs->timestamp) {
        return 0;
    } 
    return 1;
}

static void timer_base_handler(int signum, siginfo_t* info, void* ucontext) {
    if (signum != SIGALRM) {
        printf("Receive Other signal:%d, just ignore.\n", signum);
        return;
    }
    // TODO Find the smallest timestamp node and run it's handler.
    printf("Receive alarm.\n");
}

int init_timer_context(struct timer_context* context, int max_timer_count) {
    context->nodes = (struct timer_node*)malloc(sizeof(struct timer_node) * max_timer_count);
    if (context->nodes == NULL) {
        printf("Allocate memory for context failed!\n");
        return -1;
    }
    context->capacity = max_timer_count;
    context->size = 0;
    context->act.sa_sigaction = timer_base_handler;
    context->act.sa_flags = SA_SIGINFO;
    context->base_timeval.it_interval.tv_sec = 1;
    context->base_timeval.it_interval.tv_usec = 0;
    context->base_timeval.it_value.tv_sec = 1;
    context->base_timeval.it_value.tv_usec = 0;

    if (sigaction(SIGALRM, &context->act, NULL) != 0) {
        printf("Register signal action failed!\n");
        return -1;
    }
    if (setitimer(ITIMER_REAL, &context->base_timeval, NULL) != 0) {
        printf("Start base timer failed!\n");
        return -1;
    }
    return 0;
}

void uninit_timer_context(struct timer_context* context) {
    if (context == NULL) { return; }
    free(context->nodes);
    context->nodes = NULL;
    context->capacity = 0;
    context->size = 0;
    context = NULL;
}

/**
 * \brief Create a timer.
 *
 * @param [in] handler: Timer process functions when invoking.
 * @return return timerid if success, return -1 if error.
 *
 */
int create_timer(timer_handler handler) {
}

/**
 * \brief Start/stop a timer.
 *
 * @param [in] timerid: Timer identifier.
 * @return 0:success -1:error
 */
int set_timer(int timerid, int seconds);

int main() {
    // Init timer context .
    struct timer_context context;
    int max_timer_count = 100;
    if (init_timer_context(&context, max_timer_count) != 0) {
        printf("Initialize timer context failed!\n");
        return -1;
    }
    printf("Timer init success.\n");

    // create_timer()


    // set_timer()

    // get_timer()

    // delete_timer()

    int counter = 0;
    while (counter < 10) {
        pause();
        counter++;
    }

    // Unit timer context.
    uninit_timer_context(&context);
    printf("Timer uninit success.\n");
    return 0;
}

