#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
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
    int signature;

    struct sigaction act;
    struct itimerval base_timeval;
};

int timer_node_compare(const struct timer_node* lhs, const struct timer_node* rhs);
void timer_node_swap(struct timer_node* lhs, struct timer_node* rhs);


int init_timer_context(struct timer_context* context, int max_timer_count);
void uninit_timer_context(struct timer_context* context);

/**
 * \brief Create a timer and process.
 *
 * @param [in] context: Context to add timer.
 * @param [in] handler: Timer process functions when invoking.
 * @param [in] seconds: Timer timeout in seconds.
 * @return return timerid if success, return -1 if error.
 *
 */
int create_timer(struct timer_context* context, timer_handler handler, int seconds);
static void timer_base_handler(int signum);

static void timer_handler_1(int signum) {
    printf("Timer handler 1 process\n");
}
static void timer_handler_2(int signum) {
    printf("Timer handler 2 process\n");
}
static void timer_handler_3(int signum) {
    printf("Timer handler 3 process\n");
}
static void timer_handler_4(int signum) {
    printf("Timer handler 4 process\n");
}
static void timer_handler_5(int signum) {
    printf("Timer handler 5 process\n");
}

static struct timer_context* g_timerContext = NULL;
static int g_loopFlag = 1;

int main() {
    // Init timer context .
    struct timer_context context;
    g_timerContext = &context;
    int max_timer_count = 100;
    if (init_timer_context(&context, max_timer_count) != 0) {
        printf("Initialize timer context failed!\n");
        return -1;
    }
    printf("Timer init success.\n");

    // Create several timers.
    create_timer(&context, timer_handler_1, 2);
    create_timer(&context, timer_handler_2, 4);
    create_timer(&context, timer_handler_3, 6);
    create_timer(&context, timer_handler_4, 8);
    create_timer(&context, timer_handler_5, 4);

    while (g_loopFlag) {
        pause();
    }

    // Unit timer context.
    uninit_timer_context(&context);
    printf("Timer uninit success.\n");
    return 0;
}

int timer_node_compare(const struct timer_node* lhs, const struct timer_node* rhs) {
    if (lhs->timestamp < rhs->timestamp) {
        return -1;
    } else if (lhs->timestamp == rhs->timestamp) {
        return 0;
    } 
    return 1;
}

void timer_node_swap(struct timer_node* lhs, struct timer_node* rhs) {
    if (lhs == rhs) { return; }

    struct timer_node temp;
    temp.handler = lhs->handler;
    temp.timerid = lhs->timerid;
    temp.timestamp = lhs->timestamp;

    lhs->handler = rhs->handler;
    lhs->timerid = rhs->timerid;
    lhs->timestamp = rhs->timestamp;

    rhs->handler = temp.handler;
    rhs->timerid = temp.timerid;
    rhs->timestamp = temp.timestamp;
}

static void timer_base_handler(int signum) {
    if (signum != SIGALRM) { 
        g_loopFlag = 0;
        return;
    }
    printf("In base handler\n");

    struct timer_context* context = g_timerContext;
    if (g_timerContext->size <= 0) { return; }

    // We use min heap to organize timers, so just check the first timer timeout state.
    time_t current_timestamp = time(NULL);
    struct timer_node* first = &context->nodes[0];
    if (first->timestamp > current_timestamp) { return; }

    // Process timer event.
    first->handler(signum);

    // Delete first node;
    struct timer_node* last = &context->nodes[context->size - 1];
    timer_node_swap(first, last);
    context->size--;

    // Update heap.
    int pos = 0;
    while (2 * pos + 1 < context->size) {
        int left_child_pos = 2 * pos + 1;
        int right_child_pos = 2 * pos + 2;

        int target_pos = -1;
        if (right_child_pos < context->size) {
            if (timer_node_compare(&context->nodes[left_child_pos], &context->nodes[right_child_pos]) < 0) {
                target_pos = left_child_pos;
            } else {
                target_pos = right_child_pos;
            }
        } else {
            target_pos = left_child_pos;
        }

        if (timer_node_compare(&context->nodes[pos], &context->nodes[target_pos]) <= 0) {
            break;
        } 

        timer_node_swap(&context->nodes[pos], &context->nodes[target_pos]);
        pos = target_pos;
    }
}

int init_timer_context(struct timer_context* context, int max_timer_count) {
    context->nodes = (struct timer_node*)malloc(sizeof(struct timer_node) * max_timer_count);
    if (context->nodes == NULL) {
        printf("Allocate memory for context failed!\n");
        return -1;
    }
    context->capacity = max_timer_count;
    context->size = 0;
    context->signature = 1;
    context->act.sa_handler = timer_base_handler;
    context->act.sa_flags = 0;
    context->base_timeval.it_interval.tv_sec = 1;
    context->base_timeval.it_interval.tv_usec = 0;
    context->base_timeval.it_value.tv_sec = 1;
    context->base_timeval.it_value.tv_usec = 0;

    if (sigaction(SIGALRM, &context->act, NULL) != 0) {
        printf("Register signal action failed!\n");
        return -1;
    }
    if (sigaction(SIGINT, &context->act, NULL) != 0) {
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

int create_timer(struct timer_context* context, timer_handler handler, int seconds) {
    if (context == NULL || handler == NULL) { return -1; }
    if (context->size >= context->capacity) { return -1; }

    // Create timer will change min heap structure in timer context, so
    // it should not be interrupted by signal. Here we block all signals
    // when create timer.
    sigset_t set;
    if (sigfillset(&set) < 0) {
        printf("Fill sig set failed!\n");
        return -1;
    }
    if (sigprocmask(SIG_BLOCK, &set, NULL) < 0) {
        printf("Block signal failed!\n");
        return -1;
    }

    // Create timer node.
    int timerid = context->signature++;

    int pos = context->size;
    struct timer_node* new_node = &context->nodes[pos];
    new_node->handler = handler;
    new_node->timerid = timerid;
    new_node->timestamp = (time_t)(time(NULL) + seconds);

    // Put timer node to min heap.
    while (pos > 0) {
        int parent_pos = (pos - 1) / 2;
        struct timer_node* parent_node = &context->nodes[parent_pos];
        if (timer_node_compare(new_node, parent_node) >= 0) {
            break;
        }

        // Swap if current node's timestamp < parent's timestamp.
        timer_node_swap(new_node, parent_node);
        pos = parent_pos;
    }
    context->size++;

    // Unblock signals.
    if (sigprocmask(SIG_UNBLOCK, &set, NULL) < 0) {
        printf("Unblock signal failed!\n");
        return -1;
    }

    return new_node->timerid;
}