# 15.1
这个题目的代码很简单，父进程读取文件，然后通过管道写入到子进程的标准输入。子进程中通过`exec()`打开一个分页程序，对标准输入进行分页。

如果没有这个`close()`那么子进程将一直阻塞在`read()`，导致整个进程是阻塞的，不会退出。

# 15.2
如果没有这个`waitpid()`，意味着父进程调用`close()`后进程就直接结束，不再等待并处理子进程的返回状态。子进程将由init进程收养并处理返回状态，没有什么特别大的影响。原书中的答案说由于父进程退出时shell可能会改变tty的模式，这会影响子进程的page处理，这也可能是一个点，但是和shell、page程序的行为是强相关的，这里就不做实验了。

# 15.3
代码见[15_3](./15_3.c)，我们可以发现`popen()`执行成功了，只不过因为命令不存在，所以`pclose()`返回值不是0。

为什么`popen()`执行是成功的呢？其实我们分析一下`popen()`的底层实现就知道了，它是调用了`pipe()` + `fork()`去启动了一个shell，然后在shell里面执行命令。所以`popen()`执行成功就意味着执行shell的过程是成功的，只不过shell执行命令失败了，所以`popen()`返回的fp不是NULL。

# 15.4
协同进程其实就是开了两个管道，这个概念应该是和`popen()`对应的，我们知道`popen()`只有一个单向的管道，只能单方面read或者write，这里这个协同进程的概念就是，A可以使用B作为协同进程，然后A把标准输入通过管道发给B，然后A从B的标准输出中读，作为自己的标准输出。这样A其实就成为了一个调度器，通过更换不同的B，可以实现不同的功能。

回到这道题本身，当我们终止子进程的时候，子进程的管道的fd会close，这时候父进程再进行read或者write的时候，会产生`SIGPIPE`信号，在书中的示例中通过注册信号处理程序能够捕捉到这种信号，那么如果我们去掉这个信号处理程序如何捕捉到这种情况呢？

通过`man 7 signal`我们可以发现`SIGPIPE`的默认处理方式是Terminate，程序会终止。进程无论如何终止，都会执行同一段代码，这段代码会对进程的状态进行设置，从而让别人知道进程是如何终止的。在shell中执行`echo $?`，我们就能得到父进程的终止状态，从这个终止状态中我们可以知道进程是不是正常返回，如果不是正常返回（如由于信号异常返回），也可以知道是由哪个信号触发导致的返回。

相关的讨论可以看本书的8.6节。

# 15.5
这个代码比较简单，就是把`read`, `write`换成`fgets()`和`fputs()`，没啥技术含量，这里就不写了，注意要先用`fdopen()`将fd转换为对应的FILE*即可。

# 15.6
我们先来看一下`wait()`和`waitpid()`这两个系统调用有什么区别。
```C
pid_t wait(int *wstatus);

pid_t waitpid(pid_t pid, int *wstatus, int options);
```
根据man的描述，`wait(&status) = waitpid(-1, &status, 0)`
他们的行为都是先阻塞，直到有子进程改变状态为止（默认情况下是等待子进程返回，但是可以通过option来修改），最后还会回收进程的资源（没有被wait过，却又终止的进程会变成zombie）。wait是任意一个子进程改变状态就解除阻塞，waitpid会指定一个pid进行等待。

回到这个题目本身。
`popen()`底层是`pipe()` + `fork()`产生一个shell，并在shell里面去执行cmd。
`system()`底层是`fork()` + `exec()` + `waitpid()`，会等待子进程执行完毕后再返回。

那么问题来了，当`popen()`创建的子线程先结束的时候，`system()`中如果不是`waitpid()`，而是`wait()`，那么可能会将`popen()`创建的子进程释放了，这时候我们去调用`pclose()`就没办法获得`popen()`创建的子进程的真实返回状态了。

最后再总结一下，我们之所以需要一个精准的进程等待机制，或者说是进程回收机制，就是因为一个父进程可能创建多个子进程，而在有些情况下，我们希望能够更加精确地去组织子进程的状态检查，如果只有`wait()`，我们就没办法判断当前终止的是哪个子进程。

# 15.7
这个题目的翻译不太好，其实就是想问，当我们关闭pipe的一端，然后用`select()`或者`poll()`去监测另一端，会得到什么结果。

这个答案的结果应该也和系统相关，代码见[15_7](./15_7.c)
运行这个程序，我们得到了这样的结果：
```shell
./15_7 0
===========select read test============
Write fd is marked readable.
Read size:14 msg:hello world!

===========poll read test============
Write fd is marked readable.
Read size:14 msg:hello world!

./15_7 1
===========select write test============
Write fd is marked writable.
Receive signal:13 msg:Broken pipe
write_size:-1
===========poll write test============
Write fd is marked writable.
Receive signal:13 msg:Broken pipe
write_size:-1

```
结果表明，在我这台机器上，当关闭管道的一端时，另一端可以通过`select()`或者`poll()`正常响应（能够在对应的标志位上置位）。

但是实际的读写却不是这样，当关闭了管道的写端后，如果管道中还有数据，可以继续读；然而关闭了管道的读端后，向管道写就会触发`SIGPIPE`信号。

# 15.8
我们先复习一下`popen()`的api：
```C
FILE *popen(const char *command, const char *type);
```
这里的type只能是"r"或者"w"（在glibc 2.9后还有"e"，这里不做讨论），这个type是指示的返回的FILE*的属性，是可读还是可写。

在 man 3 popen里面对返回的这个FILE*有这样的描述：
```
The  return value from popen() is a normal standard I/O
stream in all respects save that it must be closed with
pclose() rather  than  fclose(3).

Writing to such a stream writes to the standard input of
the command; the command's standard output is the same as
that of the process that called popen(), unless this is
altered by the command itself. Conversely, reading from
the stream reads the command's standard  output,  and 
the  command's  standard input is the same as that of 
the process that called popen().

```
这段话的意思是这样的，当type = "r"时，表明我们只能从这个FILE*中读，读到的是子进程中stdout的输出。子进程的stdin和父进程的stdin一致。

当type = "w"时，表明我们只能向这个FILE* 中写，写入这个FILE* 的内容会写到子进程的stdin中，子进程的stdout和父进程的stdout一致。

而其他的描述符，是不做处理的，所以根据`fork()`的规则进行继承，只要子进程没有重定向，那么子进程和父进程的fd是指向同一个打开文件的。

代码见[15_8](./15_8.c)，我们可以看到子进程中的stderr中写和在父进程中的stderr中写，效果是一样的。

# 15.9
这个问题我们在之前15.6题中就已经分析过了，一共三个进程：
1. 父进程
2. shell进程
3. cmd执行的进程

cmd执行完毕后，shell通过`waitpid()`回收cmd进程的资源，然后shell退出，父进程通过`waitpid()`回收shell的资源。

# 15.10
我们先简单总结一下FIFO的行为，其实FIFO的使用很简单：
1. 创建一个FIFO文件：`mkfifo()`
2. 像使用正常文件一样打开这个文件，并进行读写。

FIFO的底层结构和pipe管道很类似，都是一段内核内存。但是因为FIFO抽象成了文件，所以可以进行进程间的通信。

因为FIFO是为了进程间的通信来设计的，这就意味着FIFO正常工作的时候必然是有读端和写端。所以在默认情况下`open()`FIFO的时候会阻塞，直到读端和写端都准备好了为止。

如果我们在`open()`的时候指定了`O_NONBLOCK`标志，那么为读打开和为写打开的行为是不一样的：
+ O_RDONLY: 这时候open立刻返回，open成功，返回fd。后续可以使用write正常向FIFO中写（在FIFO缓冲区未满前）。
+ O_WRONLY: 这时候open立刻返回，open失败。
+ O_RDWR: 这时候open立刻返回，open成功，返回fd，FIFO缓冲区未满前，可以正常读写。

在我的系统上，一个O_NONBLOCK的FIFO不需要像本题答案中说的那么复杂，需要两次open，只需要一次open，并指定O_RDWR就可以了。代码见：[15_10](./15_10.c)

# 15.11
消息队列这个东西目前已经不怎么用了，它的api比较复杂，没有共享内存和FIFO用起来方便，而且以前存在的性能优势现在也没了。这个题目也很简单，如果恶意进程能读消息队列，那么就会影响正常的服务器和客户进程的通信，只要知道消息队列的key就可以读消息队列了。

# 15.12
这里因为要用到消息队列，我们还是看一下消息队列的api：
```C
// Create msg queue.
int msgget(key_t key, int msgflg);

// Control msg queue.
int msgctl(int msqid, int cmd, struct msqid_ds* buf);

// Send msg.
int msgsnd(int msqid, const void* ptr, size_t nbytes, int flag);

// Receive msg.
ssize_t msgrcv(int msqid, void* ptr, size_t nbytes, long type, int flag);
```

代码见[15_12](./15_12.c)。

# 15.13
这个题目就是想告诉我们共享内存的常见用法，因为不同的进程的地址空间是不一样的，所以在共享内存中，我们不会保存对象的指针，而是保存对象相对于共享内存起始地址的偏移。通过`reinterpret_cast<Object*>(addr + offset)`这种方式去访问具体的对象。

# 15.14
子进程和父进程之间做了同步，他们的i是各自独立变化的，由于使用了`mmap()`，所以area中的值是会被子进程和父进程交替改变的。具体的图表看书后答案即可。

# 15.15
使用共享内存来代替`mmap()`很简单，代码见[15_15](./15_15.c)

# 15.16
使用信号量来代替`mmap()`，代码见[15_16](./15_16.c)

# 15.17
使用记录锁实现，代码见[15_17](./15_17.c)

# 15_18
使用POSIX信号量来实现，代码见[15_18](./15_18.c)
