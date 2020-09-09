# 14.1
首先我们需要弄清楚fcntl记录锁的锁定规则，其实很简单，就是一个读写锁的规则：当一个字节被写锁持有时，其他进程上读锁或者写锁都会失败；当一个字节被读锁锁住时，其他进程可以上读锁，不能上写锁。

这个题目问的是，如果一个文件不断被上读锁，那么阻塞的写锁请求会不会被饿死。为什么会问这个问题呢？因为读写锁通常有一个机制，如果有一个阻塞的上写锁的请求时，后面的读锁请求会先等待，直到上写锁成功并释放锁后，才能加读锁，这种设计就是为了防止太多的读锁请求，让写锁一直得不到执行。

那么针对文件的fcntl锁有没有这样的特性呢，代码见[14_1](./14_1.c)。结果发现写锁需要等到所有的读锁释放后才能加锁成功，否则阻塞，而在写锁阻塞等待期间，读锁还可以正常加锁，所以fcntl没有这样的特性，写锁是可能被饿死的。

# 14.2
又到了翻源码的时候，如下：
```C
// #include <sys/select.h>
// 这四个宏实际上是调用了bits/select.h中的内部宏
#define	FD_SET(fd, fdsetp)	__FD_SET (fd, fdsetp)
#define	FD_CLR(fd, fdsetp)	__FD_CLR (fd, fdsetp)
#define	FD_ISSET(fd, fdsetp)	__FD_ISSET (fd, fdsetp)
#define	FD_ZERO(fdsetp)		__FD_ZERO (fdsetp

// 我的机器中fd_set的定义, 其实很简单，用一个bit来表示一个fd的状态，最多表示的fd数目为__FD_SETSIZE = 1024
// 然后一个__fd_mask就可以表示 8 * sizeof(__fd_mask)这么多个fd，我们只需要__FD_SETSIZE / __NFDBITS个__fd_mask就可以表示1024个fd的状态了
typedef long int __fd_mask;
#define __FD_SETSIZE		1024
#define __NFDBITS	(8 * (int) sizeof (__fd_mask))
typedef struct {
    __fd_mask __fds_bits[__FD_SETSIZE / __NFDBITS];
} fd_set;

// 这里是一些常用的内部操作
// 这里的输入`d`是一个fd， 因为我们用一个数组来记录所有fd_set的状态，这里就是得到`d`这个fd对应
// __fds_bits数组中的下标，然后我们就能知道是哪个__fd_mask用于保存`d`的状态。
#define	__FD_ELT(d)	((d) / __NFDBITS)

// 找到`d`在__fds_bits中的位置后，通过这个__FD_MASK确定是__fd_mask中的哪个bit用于保存`d`的状态
#define	__FD_MASK(d)	((__fd_mask) (1UL << ((d) % __NFDBITS)))

// 这就是由fd_set得到__fds_bits这个数组
# define __FDS_BITS(set) ((set)->__fds_bits)

//////////////////////////////////////////////////////////////////////////////////////////

// #include <bits/select.h>
// 用__FDS_BITS找到__fds_bits这个数组，然后__FD_ELT找到对应的__fd_mask，然后__FD_MASK找到对应bit，|=运算置为1
#define __FD_SET(d, set) \
  ((void) (__FDS_BITS (set)[__FD_ELT (d)] |= __FD_MASK (d)))

// 同上，只不过是用~先取反，然后&=，达到把对应位置0的效果
#define __FD_CLR(d, set) \
  ((void) (__FDS_BITS (set)[__FD_ELT (d)] &= ~__FD_MASK (d)))

// 同上，只不过是用&运算来检查对应位是不是1
#define __FD_ISSET(d, set) \
  ((__FDS_BITS (set)[__FD_ELT (d)] & __FD_MASK (d)) != 0

/*
 这里用了内联汇编，执行的命令有三个cld, rep和stosq
 cld: 指定DF标志的方向从低地址到高地址，这里用来设置数组遍历的方向
 rep：重复某个指定，执行一次ecx减1，直到ecx为0
 stosq: 把eax中的值写入到edi指定的内存中

 这样看来这段在干什么就很清晰了，首先把eax置为0，然后ecx置为__fds_bits这个数组的长度，edi置为数组的地址
 然后开始循环，每次循环都把eax的值赋值到edi指定的位置，然后edi+=4，ecx--，直到ecx = 0
 整个FD_ZERO的效果和下面的C语言代码一样：
 for (int i = 0; i < sizeof(fd_set) / sizeof(__fd_mask); ++i) {
     __fds_bits[i] = 0;
 }
 */
#  define __FD_ZERO_STOS "stosq"
# define __FD_ZERO(fdsp) \
  do {									      \
    int __d0, __d1;							      \
    __asm__ __volatile__ ("cld; rep; " __FD_ZERO_STOS			      \
			  : "=c" (__d0), "=D" (__d1)			      \
			  : "a" (0), "0" (sizeof (fd_set)		      \
					  / sizeof (__fd_mask)),	      \
			    "1" (&__FDS_BITS (fdsp)[0])			      \
			  : "memory");					      \
  } while (0)
```

# 14.3
通过上面这个题目我们就能知道，`fd_set`这个数据结构中能表示的最多的描述符数是用__FD_SIZE来定义的，所以我们只要改变这个宏的数值就可以了。

这个宏定义在`bits/typesizes.h`中，这里要注意一个宏展开的特点，就是文本展开，所以我们需要先改变__FD_SIZE，然后才能引用`sys/select.h`中的相关内容，而且一定要保证用到的头文件都有
```
#ifndef XX
#define XXX
// ...
#endif
```
这样的头文件保护，这才能保证不会因为头文件被重复引用而导致值又修改回去了，用法如下：
```
#include <bits/typesizes.h>
#define __FD_SIZE 2048
#include <sys/select.h>
```
一般来说，在用户层的应用代码中，是不直接用`bits/typesizes.h`这样的内部头文件的，所以这种解决方案是很丑陋的，在实际的工程使用中，更好的解决这个问题的方式就是：不要用`select`，而是用`epoll`这种更新的多路复用方案，没必要纠结一个有缺陷的工具。

# 14.4
又是看源码环节，我们先把两边的API列出来：
fd_set处理相关：
```C
// #include <sys/select.h>
int FD_ISSET(int fd, fd_set* fdset);
void FD_CLR(int fd, fd_set* fdset);
void FD_SET(int fd, fd_set* fdset);
void FD_ZERO(fd_set* fdset);
```

sigset_t相关：
```C
// #include <signal.h>
int sigemptyset(sigset_t* set);
int sigfillset(sigset_t* set);
int sigaddset(sigset_t* set, int signo);
int sigdelset(sigset_t* set, int signo);

int sigismember(const sigset_t* set, int signo);
```

fd_set的实现我们已经看过了，是用的宏来实现的，我们这次来看看信号集是如何处理的：
```C
// 首先直接看signal.h，发现上面这些函数都是定义为extern。
// 然后我们通过一番查找，发现Linux的源码通常位于/usr/src/中，于是我们找到了对应的实现：

// sigset_t的定义，_NSIG代表signal的种类数，在我的系统上为64。_NSIG_BPW的意思我猜是bits per word
// 也就是一个word能表示多少个bits，最后的sigset_t也是一个unsigned long的数组，这样我们就知道了
// 信号集和fd_set很类似，都是用一个数组来表示集合的状态，用一个bit来表示其中一个信号的状态。
#define _NSIG           64
#ifdef __i386__
# define _NSIG_BPW      32
#else
# define _NSIG_BPW      32
#endif
typedef struct {
        unsigned long sig[_NSIG_WORDS];
} sigset_t;

// sigemptyset()就是把sig这个数组置为0，所有位都置0，实现为一个inline的函数。
static inline void sigemptyset(sigset_t *set)
{
    switch (_NSIG_WORDS) {
    default:
        memset(set, 0, sizeof(sigset_t));
        break;
    case 2: set->sig[1] = 0;
    case 1: set->sig[0] = 0;
        break;
    }
}

// 类似sigemptyset，只不过把值设为-1，一个unsigned long把值置为-1，其实就是所有位都置1，这里有一/// 个隐式转换，是把long->unsigned long， (long)-1底层的bit表示就是全为1.
static inline void sigfillset(sigset_t *set)
{
    switch (_NSIG_WORDS) {
    default:
        memset(set, -1, sizeof(sigset_t));
        break;
    case 2: set->sig[1] = -1;
    case 1: set->sig[0] = -1;
        break;
    }
}

/**
 * 这里倒是有几个细节可以看一下，首先signum应该是从1开始编号的，所以这里先sig = _sig -1
 * 得到对应的下标。sig数组中一个bit表示一个信号的状态，这里用了 1UL << sig 这种写法用来
 * 给对应的bit置位。
 * 由于系统字长的不一样，所以通用的规则其实是：
 * 1. sig / _NSIG_BPW, 找到是数组中的哪个元素保存了当前sig的状态
 * 2. sig % _NSIG_BPW, 上面找到的这个数组元素中是哪个bit保存了当前的sig的状态
 * 3. set->sig[sig / _NSIG_BPW] |= 1UL << (sig % _NSIG_BPW)把对应bit置1
 */
static inline void sigaddset(sigset_t *set, int _sig)
{
    unsigned long sig = _sig - 1;
    if (_NSIG_WORDS == 1)
        set->sig[0] |= 1UL << sig;
    else
        set->sig[sig / _NSIG_BPW] |= 1UL << (sig % _NSIG_BPW);
}

// 同上，只不过这里因为要置0，所以用set->sig[sig / _NSIG_BPW] &= ~(1UL << (sig % _NSIG_BPW))的/// 方法来把对应bit置0
static inline void sigdelset(sigset_t *set, int _sig)
{
    unsigned long sig = _sig - 1;
    if (_NSIG_WORDS == 1)
        set->sig[0] &= ~(1UL << sig); 
    else
        set->sig[sig / _NSIG_BPW] &= ~(1UL << (sig % _NSIG_BPW));
}   


// 同上，只不过是用1 & (set->sig[sig / _NSIG_BPW] >> (sig % _NSIG_BPW))的方法来判断
// 对应bit是不是1
static inline int sigismember(sigset_t *set, int _sig)
{
    unsigned long sig = _sig - 1;
    if (_NSIG_WORDS == 1)
        return 1 & (set->sig[0] >> sig);
    else
        return 1 & (set->sig[sig / _NSIG_BPW] >> (sig % _NSIG_BPW));
}  

```

最后总结一下，sigset_t和fd_set操作的基本原理还是一样的，都是用一个bit表示一个特定的sig或者fd的状态，不同之处在于sigset_t使用了inline函数的方式实现，fd_set使用了宏实现，还用了点内联汇编提升性能。 

# 14.5
这个题目和之前的一个题目好像一样，代码也很简单，见[14_5](./14_5.c)。

# 14.6
这里我们先回忆一下记录锁的原理，书中图14-8很好展示了其原理。重点有以下几个：
1. 记录锁的实现是实现在v节点表中，这是一个内核级别的数据结构，也就是说是所有进程共享这个结构。这就是为什么记录锁能够跨进程进行状态同步。
2. 对于同一个文件的同一个位置的所有锁都是记录在一个`struct lockf_entry`中，所以只要一个`close(fd)`操作，就会释放这个节点对应的所有的锁。
3. 一个进程获得锁后可以重复给同一个文件段加锁而不会阻塞。

然后我们来思考一下这个题目本身，题目要求我们用记录锁来进行进程间的同步，很显然我们应该用阻塞版本的写锁，也就是`fcntl(fd, F_SETLKW, F_WRLCK)`。只要用同一个记录锁来进行同步即可，可以保证parent和child之间的同步，代码见[14_6](./14_6.c)

注意这里采用了一种不严格的方式保证父进程和子进程的执行顺序，就是让释放锁的进程`sleep(1)`， 这样使得正在阻塞等待的进程有时间可以获取这个锁，不然可能会出现一个进程不断获得锁、释放锁，另一个进程得不到执行的情况。

不用sleep的方法也有，我们可以加上文件的读写，在记录锁对应的文件中用一个字节表示当前应该执行的进程，例如0表示父进程，1表示子进程。每次获得锁之后需要先读一下文件，如果文件中的值和进程对应的编号不符，那么直接释放锁；如果相符，则执行，执行结束后，修改文件中记录的编号，这样下次就可以由另外一个进程来执行了。用记录锁来同步本来就是一个比较差的做法，我这里就不实现这个版本了。

# 14.7
这个问题很简单，我们只要不断往管道中写，不要去读，直到管道写满为止，记录一下写入的总字节数就可以了，代码见[14_7](./14_7.c)
在我的机器上，pipe的buffer实际长度是65536, 但是`PIPE_BUF`值为4096。

# 14.8
通过这个题目我们来看一下aio的相关api，目前aio的实现版本有很多，书上讲的是Posix aio，其实还有Linux native aio等等。这些aio实现的核心点有几个：
1. 提交aio请求，内核一般维护一个aio请求的队列，用户提交了aio请求后，只要这个请求入队了就可以返回，不用等待io完成。
2. 内核以某种方式监听io事件的完成，当io时间完成后能够以某种方式进行通知。
3. 对已经完成的io进行处理。

然后我们看一下Posix aio的接口：
```C
// 读，提交一个读请求到内核的队列
int aio_read(struct aiocb *aiocbp);

// 写，提交一个写请求到内核的队列
int aio_write(struct aiocb *aiocbp);

// 一次提交多个读写aiocb.
int lio_listio(int mode, struct aiocb *const aiocb_list[],
               int nitems, struct sigevent *sevp); 

// 用于轮询检查，用户检查某一个aio请求是否完成
int aio_error(const struct aiocb *aiocbp);

// 用于结果检查，只有当io完成后才能调用这个api，检查io状态是否正常。
ssize_t aio_return(struct aiocb *aiocbp);

// 阻塞机制，有点类似于select，用户同时监听多个aiocb，直到有一个io已经完成为止
// 这个api只能判断有io回来了，并不能告诉我们是哪个io完成了，所以需要自己用aio_error()
// 轮询检查。
int aio_suspend(const struct aiocb * const aiocb_list[], int nitems,
                const struct timespec *timeout);

// aio取消机制
int aio_cancel(int fd, struct aiocb *aiocbp);
```
通过这个api可以看到，Posix aio的事件响应机制其实不太好。Posix aio提供的主动响应机制是利用信号实现的，而信号是可能丢失的，稳定性得不到保证，另外对于一个io请求，需要用aio_error() + aio_return()两个调用去验证这个aio请求是否完成，有时候还要轮询aio请求找到完成的请求，效率是比较低的，所以一般工程中都不用这一套Posix的aio接口。

回到这个题目本身，这个题目的要求其实有点奇怪，我们要从stdin读，然后写入到stdout。stdin/stdout是流式的输入输出，对于这种流式的文件，一般是用不了aio的，因为我们不知道流式文件的大小，而且stdin也不支持`lseek()`操作，我们不能从中间截取一段开始读。

如果强行要用aio的话，我们只能使用一个aiocb来进行读写，但是这样aio就没有多路复用的优势了。总之这个题目很奇怪，但是我还是勉强把答案写出来了，代码见[14_8](./14_8.c)。

# 14.9
这里出现的场景是多块buffer需要写入到同一个文件时，两种方案的开销：
1. 首先先申请一块大的buffer，然后把所有的buffer拷贝到大buffer中，最后执行一次write。
2. 调用writev

我们可以分析一下方案1和方案2各需要执行些什么操作。
方案1：一次malloc + N次memcpy + 1次write。其中malloc和memcpy是库函数调用，write是系统调用，会陷入内核。

方案2：writev一次系统调用，通过查看[writev源码](https://github.com/torvalds/linux/blob/master/fs/read_write.c)，我们知道其实writev底层就是遍历了所有的`struct iovec`然后执行了写入。

对比发现，当写入块比较少的时候，writev由于内部的检查更多，调用链更长，所以可能耗时更长，但是随着写入块变多，writev比方案1少了N次拷贝和1次malloc，所以理论上会性能更好。

具体的测试数据我这里就不做了，和机器的配置是强相关的，只要知道这个趋势就好了。

# 14.10
需求很简单，代码见[14_10](./14_10.c), 在我的机器上，执行完拷贝之后使用`stat`命令查看源文件的access time，发现access time并没有更新。

# 14.11
我们直接复用一下[14_10](./14_10.c)的代码，去掉注释即可。可以发现确实调用`mmap()`后关闭输入文件，内存映射依然不会失效。
