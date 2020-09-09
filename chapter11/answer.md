# 11.1
图11.4中的代码存在的问题是，将一个线程中的局部变量的地址返回了，然后在另一个线程中希望用这个地址能够获取到结构的值，这种行为是未定义的。

如果我们希望在线程之间传递数据的话，一般使用堆内存，也就是`malloc()`出来的内存，代码见[11_1](./11_1.c)。

本书的答案中把线程的返回状态用来传递这个参数了，实际上是不对的，因为这个值本来是应该反映线程的退出状态的，通过看`pthread_join()`的api，我们也能看出这一点。

所以如果要在线程之前传递变量，应该是利用线程在创建时的`arg`参数来进行传递，最好不要用返回值来传递。

```C
int pthread_join(pthread_t thread, void **retval);

/*
If retval is not NULL, then pthread_join() copies the exit status of the target thread (i.e., the  value  that  the  target
thread  supplied  to  pthread_exit(3))  into  the  location  pointed to by retval.  If the target thread was canceled, then
PTHREAD_CANCELED is placed in the location pointed to by retval.
*/
```

# 11.2
首先我们理解一下图11-14在干什么，其实很简单，就是一个读写锁最简单的应用。
每一个任务是一个`struct job`，多个任务形成一个`struct queue`，主线程可以创建任务，并向这个queue中插入：`job_insert()`和`job_append()`，其他线程是消费者，首先搜索这个队列：`job_find()`，然后从队列中取出任务`job_remove()`并执行。

根据这个api，我们发现任务一旦被创建，加入队列后，就已经确定了job id，而本题希望我们能够在job被加入队列后，还没有被消费前，能够改变job的id（是直接改变，而不是先remove再insert），所以我们需要实现一个`job_change()`，找到第一个job id，并把job id修改为其他的job id。如果要修改job id首先我们要获取queue的读锁，这样我们才能在queue中找到这个job，但是这样还不够，由于我们要修改job id，为了防止别的线程在我们修改的时候去check job id，我们还要给job这个结构上一个互斥锁。

代码见[11_2](./11_2.c)，为了测试`job_change()`，我们定义了一个lazy_thread和hardworking_thread。lazy thread会把自己的所有任务都转给hardworking thread去执行。

由于`job_remove()`执行时给job queue上了一个写锁，所以`job_change()`没有给`job_remove()`带来额外的修改。

# 11.3
通过这个题目我们来好好理解一下条件变量和互斥量分别解决了什么问题。

首先我们看互斥量，互斥量最典型的用法是：
```C
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_lock(&lock);
// process
pthread_mutex_unlock(&lock);
```
如果有线程A和B竞争同一个互斥量，那么这个互斥量能够保证的是：**同一时刻只有一个线程能够进入临界区**。至于是A先进入临界区，还是B先进入，互斥量是没办法做这个保证的。所以有了互斥量，我们仅仅能够确保多线程任务对于临界区的修改是原子的，不会因为多线程的原因丢失修改。

然后我们就可以看条件变量，条件变量最典型的用法是：
```C++
pthread_mutex_t lock = PTHERAD_MUTEX_INITIALIZER;
int flag = 0;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

// In thread A.
pthread_mutex_lock(&lock);
while (flag == 0) { 
    pthread_cond_wait(&cond, &lock);
}
flag = 0;
pthread_mutex_unlock(&lock);
// Process A.

//=====================================================================

// In thread B.
// Process B.
pthread_mutex_lock(&lock);
flag = 1;
pthread_mutex_unlock(&lock);
pthread_cond_signal(&cond);
```
条件变量的目的就是为了解决线程之间的**执行顺序**问题，比如A线程需要等待B线程执行一些处理后，才能开始执行，所以我们需要有一个阻塞机制，当A线程要执行自己的任务，但是条件还不满足的时候，这时候A线程应该要阻塞，直到条件被满足。

条件变量的用法看起来比较复杂，一开始会不太清楚为什么要这么写，我们分析一下。

在上面这个例子中，`flag`这个变量才是条件变量，为什么这么说呢？因为线程A是在等待`flag != 0`，所以当`flag == 0`的时候就表明线程A的条件还没有被满足，这时候线程A需要等待，所以才调用`pthread_cond_wait()`进行阻塞,`pthread_cond_wait()`只是阻塞的一种手段而已，`flag`才是线程A等待的条件。

然后我们围绕`flag`来看，为什么线程A需要一个互斥锁`lock`呢，因为我需要对条件`flag`进行判断，`while (flag == 0)`这个表达式并不是原子的（执行的时候至少需要mov + 一个test指令），而其他线程（例如B），又需要改这个`flag`，所以我们需要用一个互斥锁锁住这个`flag`从而保证同步。重点：**互斥锁lock是用来锁住flag以保证同步的**。

然后我们看`pthread_cond_wait(&cond, &lock);`这一句，传入了两个参数`cond`和`lock`，这个`cond`我们可以理解，因为一个进程里面可能有多个条件变量，所以`cond`这个变量是为了标记当前等待的是哪一个条件变量。

那么为什么有一个`lock`呢？其实我们看一下`pthread_cond_wait()`这个函数的行为就知道了，如果被阻塞，那么会释放这个`lock`，然后再次被唤醒时，又会重新获得这个`lock`。从上面我们已经知道了，为了保证条件`flag`的原子性，我们必须要用一个互斥量`lock`包裹这个条件，而在条件不满足的时候，我们需要`pthread_cond_wait()`进入阻塞，**而这时候我们还没有释放这个互斥量**。如果我们就直接进入阻塞的话，那么会因为无法获得互斥量`lock`，从而没办法改变条件并执行`pthread_cond_signal()`，这样就导致了死锁。所以这个传入的`lock`就是为了能够在`pthread_cond_wait()`内部释放`lock`，并在唤醒时重新获得`lock`，**是为了防止死锁的一种简便的实现**。如果我们在`pthread_cond_wait()`中不传入这个`lock`，也是可以实现的，但是对于调用方来说，又会复杂一些，我们需要这样调用：
```C
pthread_mutex_lock(&lock);
while (flag == 0) { 
    // Unlock.
    pthread_mutex_unlock(&lock);
    pthread_cond_wait(&cond);
    pthread_mutex_lock(&lock);
}
flag = 0;
pthread_mutex_unlock(&lock);
```
无论如何我们需要保证两件事：
1. `flag`在读、写、测试的时候要被互斥量包裹。
2. 被`pthread_cond_wait()`阻塞当前线程的时候需要释放当前互斥量，不然其他线程就没办法进入临界区修改条件。

回到这道题本身，不得不吐槽一下，这本书里面的习题翻译得是真的差，我翻看了后面的答案，想了好一会才明白表达的是什么意思。其实很简单，在11_2中，我们只是为了展示job的insert, remove, change的过程，所以我们的工作线程在process的时候，使用了这样的方式:
```C++
while (1) {
    job_find();
    job_remove();
    job_process()
    sleep(1);
}
```
注意到这里有一个`sleep(1)`，如果我不加这一行，那么当`job_queue`中没有任务的时候，工作线程就处于busy waiting的状态，CPU会直接跑到100%，特别是当线程越多的时候，理论上N个线程，CPU就会是N * 100%，这样白白消耗CPU在工程上是不能接受的，因为你会挤占其他任务的执行。

而题目里面的意思就是，让工作线程在没有任务的时候使用条件变量去wait，直到主线程往`job_queue`里面insert后，才解除阻塞。

这里书里面还问了一下，存在什么问题，其实是作者预想我们会实现一个比较愚蠢的版本，所有线程都用同一个条件变量去监控，这样会有**惊群效应**，也就是我的`job_queue`只插入了一个job，但是却唤醒了wait的所有线程，导致除了抢到任务的那个线程外，其他线程先唤醒，又马上因为没抢到任务又阻塞了。

其实我们只要给每个工作线程都用一个条件变量就能解决这个问题，主线程向job_queue中添加线程A的任务时，只唤醒线程A即可。

当然，如果要考虑到`job_change`的话，问题就复杂一些，我们这里就不考虑`job_change`了，代码见[11_3](./11_3.c)

# 11.4
这两种序列的差别就只有一个：要不要把`pthread_cond_broadcast()`包裹在条件变量的互斥锁内部。也就是锁的粒度的问题。从原理上来说，互斥量应该只需要保护条件变量的条件即可，不需要把`pthread_cond_broadcast()`包含在锁中，但是一般来说锁的粒度大一些，也不会影响正确性，只会对性能造成影响，我们来尝试分析一下：

首先我们要明确工作线程是如何进行处理的，工作线程的处理应该和我们上面的写法类似：
```C
pthread_mutex_lock(&lock);
while (flag == 0) { 
    pthread_cond_wait(&cond, &lock);
}
flag = 0;
pthread_mutex_unlock(&lock);
```
注意这里我们必须要在一个while循环中检查条件，因为同一个时刻可能有多个线程竞争同一个锁，只会有一个工作线程能够竞争到这把锁，其他没有竞争到锁的线程，需要继续阻塞在`pthread_cond_wait()`，如果这里写一个`if (flag == 0)`，那么这个线程只要阻塞了一次，下次唤醒的时候就会继续往下执行了，会带来正确性的问题。

然后我们梳理一下工作线程在进入阻塞后，到收到唤醒的信号重新执行的过程：
1. 由于`pthread_cond_wait()`进入阻塞。
2. 收到`pthread_cond_broadcast()`的信号，线程重新进入调度队列。
3. 尝试获取`pthread_mutex_lock()`，如果获取lock，则从`pthread_cond_wait()`中返回，否则继续睡眠。
4. 只要从`pthread_cond_wait()`中返回，则说明此工作线程已经通过竞争获得了锁，再进行一次`while (flag == 0)`的判断（代码实现正确的话必然不会进入wait），进入下面的步骤。
5. 修改flag，解锁。

基于上面的分析，我们对这两种序列进行分析：

第一种：先`pthread_cond_broadcast()`再解锁。如果工作线程收到了`pthread_cond_broadcast()`的信号，被重新调度，尝试获取锁时，主线程已经完成了解锁，那么工作线程就能顺利获得锁并执行，效果就是主线程的`pthread_cond_broadcast()`和工作线程的调度->获取锁这个过程是并行的。

反之如果工作线程尝试获取锁的时候，主线程还没有完成解锁，那么此时工作线程会再次进入阻塞，知道主线程完成解锁后，工作线程又会唤醒，并尝试获取锁。这里额外的开销就是工作线程额外多了一次唤醒->阻塞的过程。

第二种：先解锁，再`pthread_cond_broadcast()`，这时候工作线程只会在`pthread_cond_broadcast()`之后才会被唤醒，而被唤醒的时候，因为主线程已经解锁了，所以一定会有一个工作线程能够获取到这个锁，并且继续执行。

所以，这两种调用序列从正确性上来讲都是正确的，只不过第一种调用序列可能带来性能的提升或者下降。具体的性能数据，就只能通过实验来获得了，和程序本身、运行环境等因素都有较大相关，只能通过实验来决定要使用哪一种调用序列了。

# 11.5
首先我们分析一下屏障的行为，屏障是用于同步多个线程的一种方式，所有线程用同一个屏障变量`pthread_barrier_t`来同步信息，当阻塞在屏障变量上的线程达到预定的数目的时候，所有的线程同时解除阻塞，继续运行。

我们可以用一个条件变量加上计数器来模拟一个barrier，代码见[11_5](./11_5.c)

这里我们实现barrier的时候，在判断条件变量的时候没有用while语句而是用的if，这是因为barrier的语义表明，只要工作线程被唤醒，就应该继续向下执行，所以我们不用再判断条件了。


