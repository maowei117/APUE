# 17.1
借助这个题目我们首先了解一下UNIX域套接字的含义，socket我们用得最多的情况是用于网络通信，这时候的域一般是AF_INET(IPV4)或者AF_INET6(IPV6)。与之对应的还有UNIX域(AF_UNIX)，通过`socketpair()`我们能创建一对建立好连接的socket fd，它们可以类似管道一样通信，而且是全双工的。

回到这个题目上来，由于XSI的IPC使用了独立的API，所以不能使用通用的IO复用方式，例如：`epoll()`、`poll()`、`select()`，所以题目中利用了UNIX域套接字，每个线程首先`msgrcv()`接收消息，然后把消息`write()`到socket中，这样另一端就可以利用`epoll()`等方式进行监听。

这种方式的弊端在于：多了两次重复的拷贝，第一次把消息队列的数据拷贝到socket buffer中，第二次从socket buffer中拷贝出来。其实没必要把原始数据进行拷贝，只需要往管道中写一个byte来通知epoll有新的消息，触发处理逻辑即可。

代码见[17_1_sender](./17_1_sender.c)和[17_1_receiver](./17_1_receiver.c)。

这个题目要求要对消息队列做多路复用，所以我们用了UNIX域socket来完成这个功能，但是其实我们可以发现，这个多路复用还是很鸡肋的。因为每个消息队列需要一个线程把消息写入到UNIX域socket中。这就意味着当消息队列多的时候，线程数也会很多，性能也就很差了。另外子线程还需要等待主线程处理了msg之后才能接收下一个msg，所以还需要对线程之间做同步，这样的效率就更低了。

# 17.2
借助本题我们梳理一下文件描述符传送的整个过程是如何实现的。
其实核心很简单，就是通过`sendmsg()`传送了控制信息，更加进一步的来说，就是设置了`msghdr->msg_control->cmsg_type = SCM_RIGHTS;` 这样操作系统就知道我们要在这个socket上传送一个文件描述符。
```C
ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags);

ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags);

struct msghdr {
    void         *msg_name;       /* Optional address */
    socklen_t     msg_namelen;    /* Size of address */
    struct iovec *msg_iov;        /* Scatter/gather array */
    size_t        msg_iovlen;     /* # elements in msg_iov */
    void         *msg_control;    /* Ancillary data, see below */
    size_t        msg_controllen; /* Ancillary data buffer len */
    int           msg_flags;      /* Flags (unused) */
};

struct cmsghdr {
    socklen_t cmsg_len;
    int cmsg_level;
    int cmsg_type;
    /* followed by unsigned char cmsg_data[]*/
}
```

我们发现在`sendmsg()`中，既可以通过`msg_iov`传送buffer中的数据，也可以通过`msg_control`来传递控制信息，我们需要先做个实验验证一下`recvmsg()`是如何接收fd中的数据的，代码见[send_recv_msg_test](./send_recv_msg_test.c)

从这个实验里面我们就能完全弄明白`sendmsg()`是如何工作的：
1. `send_msg()`除了可以像`send()`, `sendto()`一样传输普通数据外，还可以传输控制数据，控制数据和普通数据之间是相互不干涉的，也就是说控制数据不会占用socket的payload。
2. 通过`man cmsg`我们可以看到控制信息的具体用法，需要利用下面这几个宏去操作cmsg。
```C
// 找到msghdr结构中的第一个struct cmsghdr，也就是msg_control字段
struct cmsghdr* CMSG_FIRSTHDR(struct msghdr* hdr);

// 用于遍历，找到下一个struct cmsghdr.
struct cmsghdr* CMSG_NEXTHDR(struct msghdr* hdr, struct cmsghdr* cmsg);

// 计算被align后的长度。
size_t CMSG_ALIGN(size_t length);

// 计算在length负载下，cmsg整个结构需要的buffer长度(包含了align的长度)。
size_t CMSG_SPACE(size_t length);

// 计算在length负载下，cmsg整个结构的实际长度。
size_t CMSG_LEN(size_t length);

// 找到cmsghdr对应的data结构。
unsigned char* CMSG_DATA(struct cmsghdr* cmsg);
```
 代码见[17_2](./17_2.c)

# 17.3
这是一个C语言的常识问题，全局变量的定义和声明的区别。
我们知道C语言采用了分离编译，诸如函数、全局变量，会先声明，然后在另外的文件中定义。
声明其实是告诉了编译器有这个符号，链接时要能够找到这个符号才能编译通过。
而定义就是这个符号真正的实现，会为符号分配存储空间。

例如：
```C
// 这是一个声明，声明了一个全局变量debug，它的类型是int，但是在本编译单元中
// 并不会为这个debug分配空间，在最后链接的时候，编译器需要能够找到定义好的debug
// 才能编译通过。
extern int debug;
```

```C
// 这是一个定义，在当前的编译单元中实际为debug分配了空间，其他的编译单元
// 如果声明了debug，最后会链接到这个变量。
int debug;
```

# 17.4
`buf_args()`这个代码比较简单，就是对一个字符串进行parse，得到argc和argv的过程。
书中直接用这种方式限制了argv最大长度：
```
#define MAXARGC
char* argv[MAXARGC];
```

题目要求我们要把argv这个定长数组变成一个动态长度的数组，这里我们采用了一种简便的算法，通过`realloc()`在buffer长度不够的时候进行动态分配，代码见[17_4](./17_4.c)

# 17.5
使用`poll()`的版本和使用`select()`的版本其实本质都差不多，根据书后面答案，确实有一个优化点：两个版本中都对整个client集合进行了遍历，其实我们只需要遍历有请求的client即可。但是因为`select()`和`poll()`无法得知具体哪个client有请求，所以最好的优化还是用`epoll()`代替他们。

除此之外，如果还是坚持使用`select()`和`poll()`的话，我们可以尽早退出处理client的循环，也就是在这一批有数据的client都处理完毕后直接退出client的处理循环，以`select()`版本为例：

```C
fd_set rset;
int n = select(max_fd + 1, &rset, NULL, NULL, NULL);

int counter = 0;
for (int i = 0; i < maxi; ++i) {
    if ((clifd = client[i].fd) < 0) { continue; }

    if (FD_ISSET(clifd, &rset)) {
        // Process...

        // Here if processed client fd > n, just break client fd loop.
        if (++counter >= n) { break; }
    }
}
```
这里其实还需要考虑server_fd也占了一个select的位置，所以如果接收到listen_fd的请求，同样要将counter++。

# 17.6
1. 首先`stat()`是跟随符号链接的，所以如果对应的文件是符号链接的话，得到的文件类型会是指向文件的类型。也就是说如果一个符号链接指向了非socket文件，就删不掉。这一点可以使用`lstat()`来解决。

2. 另外像这种先判断再操作的方式，始终存在一个原子性的问题，即`lstat()`和`unlink()`中间可能因为其他操作导致异常情况。例如一开始`lstat()`发现没有对应的文件，但是在`unlink()`之前，别的进程创建了一个文件，这就导致误删了别的进程的文件。一般这种情况没有特别好的解决方法，只能按照一定的规则创建一个特别的文件，例如用一个通用的key，通过md5得到一个文件名，这样就可以尽量避免冲突了。

# 17.7
回想我们在17_2中做的实验，我们当时是把要传送的fd放在`struct cmsghdr*`后的内存中的，所以我们可以有两个选择：
1. 一个cmsghdr后保存多个要传送的fd，一次`sendmsg()`只发送一个cmsghdr。
2. 一个cmsghdr后保存一个fd， 一次`sendmsg()`发送多个cmsghdr。

根据上面的思路我们实现`send_fds()`和`recv_fds()`代码见[17_7](./17_7.c)
在我的机器上，只有第一种方法可以正常工作，第二种在send的过程中会报错。