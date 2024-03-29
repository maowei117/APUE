# 10.1 
这道题目是想说明，阻塞的系统调用会被信号唤醒，如果`pause()`没有写在循环中，那么第一次收到信号并处理后，会解除阻塞并从main函数返回，这样就没办法捕捉后续的信号了，测试代码见[10_1](./10_1.c)。

# 10.2
这个题目就是单纯的工作量问题，通过一个switch语句枚举所有的信号，然后赋值对应的string即可，这里我选择偷懒，使用`string.h`中的`strsignal()`完成对应的功能，代码见[10_2](./10_2.c)

# 10.3
`signal()`这个函数作为一种不可靠信号的处理方式, 目前已经不推荐使用了. 原因有很多, 比如在多处理器架构上使用这个调用, 结果是未定义的, 而且不同平台上这个函数的行为也不一致. 

这道题让我们画出执行时的栈帧, 我们就按照作者的思路分析一下. 代码中首先注册了SIGINT和SIGALRM这两个信号的处理程序, 然后在处理SIGINT的中途触发了SIGALRM, 最终的结果是SIGALRM返回时没有继续处理SIGINT, 而是直接返回到了main函数. 所以栈帧的变化应该是这样的:
1. 刚进入main函数时, 只有main的栈帧: | main | 
2. 处理SIGINT时, 先进入了sleep2, 然后进入sig_int: | main | sleep2 | sig_int |
3. 处理SIGALRM时, 由sig_int转到了sig_alrm: | main | sleep2 | sig_int | sig_alrm | 
4. 在sig_alrm中由于使用了long_jmp, 所以并没有从sig_alrm正常返回到sig_int, 而是直接跳到了sleep2: | main | sleep2 |
5. 最终从main返回.

# 10.4
我们先看一下书中演示的正确的用法是怎样的:
```C
void sig_handler(int signum) {
    long_jmp(env_alrm, 1);
}

signal(SIGALRM, sig_handler);
if (setjmp(env_alrm) != 0) {
    alarm(10);
    // Handle timeout.
}
read();
alarm(0);
```
我们先说明一下这段代码是如何工作的, 这段代码利用了信号的机制, 当信号被处理返回时, 之前阻塞的系统调用会解除阻塞状态. 所以在read之前通过alarm(10)进行定时, 如果超过定时read()还没有返回的话, 就会由于SIGALRM信号的处理, 解除read()的阻塞. 

这里有这样几个问题:
1. 如果只是想要通过信号机制解除阻塞的话, 为什么要用long_jmp和set_jmp呢?
如果我们不用long_jmp的话, 代码如下:
```C
void sig_handler(int signum) {
    // Mark read timeout here.
    timeout = true;
}

signal(SIGALRM, sig_handler);
alarm(10);
read();
if (timeout) {
    // Process timeout here.
}
alarm(0);
```
由于进程的调度, 当系统负载比较高时, 可能的执行序列是: alarm(10) -> 切换到其他进程 -> 10s定时到, 调用sig_handler() -> read(). 在这种调用序列下, 由于alarm信号的处理先于read(), 所以这种超时机制就失效了. 发生这种情况的本质就是由于alarm信号的丢失, 导致无法影响到read()的行为了, 通过set_jmp + long_jmp的方式, 保证只要alarm触发, 一定会jump到错误处理的代码段, 信号就不会丢失了. 

2. 第二个问题就是这个题目本身了, 如果我们写代码的时候没有注意到竞争条件, 比如说像题目这样:
```C
alarm(60);
if (setjmp(env_alrm) != 0) {
    // Handle timeout.
}
```
这种写法如果调度的执行序列是: alarm(60) -> 60s定时到, 执行sig_handler(), 其中有longjmp() -> setjmp(). 就会发生先调用longjmp, 再调用setjmp()的情况. 这是未定义的, 很可能会导致程序coredump.

# 10.5 
这个题目要做得比较完备的话难度还是不小的, 其实Linux中已经有进程中的软定时器的实现了, 主要的api如下:
```C
#include <signal.h>
#include <time.h>

// 创建一个定时器
int timer_create(clockid_t clockid, struct sigevent* sevp, timer_t* timerid);

// 启动/停止定时器
int timer_settime(timer_t timerid, int flags, const struct itimerspec *new_value, struct itimerspec *old_value);

// 获取定时器剩余的时间
int timer_gettime(timer_t timerid, struct itimerspec *curr_value);

// 删除定时器
int timer_delete(timer_t timerid);
```
我们首先写个例子看一下Linux系统已有的软件定时器是怎么工作的, 然后设计我们自己的API来实现一个软件定时器. 
系统自带软件定时器的使用示例见[这里](./timer_test.c)

了解到了系统已有的软件定时器的用法后, 我们来思考一下软件定时器是如何实现的. 首先Linux为进程提供的定时机制是只支持一个定时器的, 例如`alarm()`, `setitimer()`等, 每次我们调用了新的定时器, 就会替换掉旧的定时器, 实例见[这里](./timer_test2.c).

我们自己的软件定时器应该如何设计呢? 主要的思路有这么几点:
1. 每个进程用一个有较高精度的定时器作为基础定时器, 这个定时器设定的触发时间间隔较短. 在内存中维护一个小顶堆(设定触发时间较小的在堆顶). 每次基础定时器触发时检测堆顶的事件是否已经超时, 如果超时, 则执行相应回调. 

2. 代码见[10_5](./10_5.c), 我们通过一个最小堆 + setitimer()完成了一个简易的软件定时器, 这个软件定时器的时间精度只精确到秒, 而且忽略了错误处理, 我们只需要知道这个机制即可. 为了让实现尽量简单, 这里直接使用了全局变量指向定时器的内部结构.

# 10.6
需求在题目中已经说得很清楚了, 按照要求实现即可, 代码见[10_6](./10_6.c). 这里为了让子进程先于父进程执行, 使用了两个信号flag.

# 10.7
这个题目描述的是`abort()`的一种内部实现，`abort()`的行为主要有两点要求：
1. 调用`abort()`的行为和向调用进程发送`SIGABRT`信号的行为一致，信号可以被调用线程捕捉。
2. `abort()`调用后不能返回到调用的进程，而是应该终止该进程。

这里我们一共调用了两次`kill(getpid(), SIGABRT);`, 其中第一次是因为调用进程中可能本身就设置了对信号`SIGABRT`的处理逻辑，我们希望这个处理逻辑能够得到响应。第二次是因为，`abort()`的行为要和向调用进程发送`SIGABRT`的默认处理一致（会设置进程的退出状态，是由ABRT中断导致的）。如果没有这个第二次调用，那么`abort()`的行为就是正常退出的，而不是由`SIGABRT`信号中断的了。

# 10.8
我们首先回顾一下什么是实际用户id(uid)，什么是有效用户id(euid)。
这两个概念是在进程中的概念，每个进程都有uid和euid，有这两个字段是为了表明进程所属的用户，这样在涉及到一些鉴权的地方，比如说打开文件、读写文件、发送信号的时候，就可以看当前进程是不是具有这些权限。

uid指的是调用进程的用户，在用户登录时，root用户会通过`setuid()`设置shell的uid，然后shell在exec别的可执行程序的时候，不改变uid，所以uid一般来说就和登录的用户id相同了。

euid一般情况下和uid是一样的，但是如果可执行程序文件设置了set-user-ID位，那么shell在exec这个程序的时候，会把这个进程的euid设置为文件的所有者对应的id，另外进程可以随时通过`setuid()`把euid变为uid。

然后我们来看一下发送信号所需要的权限是什么：
+ root用户可以向任何进程发送信号
+ 对于非root用户，发送者的uid或者euid必须等于接收者的uid或者euid。

然后我们再来看一下siginfo中的si_uid的目的是什么，它的目的是为了让接收信号的进程知道，是谁发送了这个信号。如果这个si_uid是一个有效id的话，譬如说这个id是root的id，那么我们无法辨别这个信号是由root用户发送的，还是由一个euid = root的进程发送过来的。所以说这个地方的si_uid是一个uid，这样就能准确表明发送信号的进程属于哪一个用户。

# 10.9
这个题目的意思我也没有太看懂，只能猜测作者想让我们干什么。首先图10-14的功能很简单，就是获取当前进程的信号屏蔽字，然后把信号屏蔽字中的信号打印出来，这里实现比较复杂的原因在于，每个信号都需要加一个if语句，所以我猜测作者应该是想让我们用一个循环来处理所有的信号，代码见[10_9](./10_9.c)，用一个数组把所有的信号和对应的message保存下来，然后用一个循环去测试，这种方法叫做**表驱动法**。这样的好处在于，我可以把这个表放在一个配置文件里面，然后进行加载，这样当我新增了一种信号需要处理时，我可以不用修改代码，而只需要去改配置就可以了，会比较灵活，代码也简洁很多。

# 10.10
这个练习是想告诉我们`sleep()`定时是不准确的，由于系统的调度需要时间，所以`sleep()`是有一定误差的，当我们运行的时间足够久的时候，就能发现累积的误差，代码见[10_10](./10_10.c)。
当我们有一个长期要运行的应用，并且依赖定时进行触发时，比如说cron守护进程、服务的日志打印等，为了能够让定时准确，我们不能使用`sleep()`这种相对时间，而是使用绝对时间，比如timestamp等。

# 10.11
代码见[10_11.c](./10_11.c), 这个题目很简单，按照题目的指示做就可以了。

# 10.12
代码见[10_12.c](./10_12.c), 在我的机器上，`fwrite()`写完了所有的数据，而且是直到文件被完全写入后，才捕捉到信号，所以我推测我这个版本的`fwrite()`实现时，先屏蔽了`SIGALRM`信号，直到写入完成后才解除屏蔽。

