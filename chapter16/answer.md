# 16.1
系统一般有两种字节序，大端序(big endian)和小端序(little endian)。
大端序是高位字节存放在低地址，小端序是低位字节存放在低地址。

需要注意的是，只有在有多个字节的时候，才存在字节序的问题，如果结构只有一个字节的话，是没有字节序的问题的，单个字节在大端序机器和小端序机器上的表示都是一样的。

判断机器是大端序还是小端序，我们只需要写一个固定的多字节值，然后判断第一个字节是最高位字节还是最低位字节就行了，代码见[16_1](./16_1.c)。

# 16.2
代码见[16_2](./16_2.c)。

# 16_3
题目的要求就是让我们做一个多端口的server，client可以通过多个端口访问访问服务。这个题目的思路也很简单，首先需要创建多个server的listen fd，然后加入到epoll中进行监听，对于到达的请求调用`serve()`响应即可。

socket的api很多，在做这个题目之前我们先进行一下梳理：

服务端建立的过程:
```C
// 1. 创建socket
// domain指定了域，比如IPV4=AF_INET,
// type指定了连接的类型，比如流式传输=SOCKET_STREAM
// 虽然目前SOCKET_STREAM 大多数都是TCP协议，但是实际上可以有别的
// 协议也是流式的，所以还有 protocol表明底层的实现协议，一般默认就填0就行了。
int socket(int domain, int type, int protocol);

// 2. 绑定一个地址。
// socket只是创建了一个fd，一个server为了让自己能够准确被外界访问，需要有一个
// 在网络中独一无二的地址，通过bind()我们就可以把socket与这个地址关联，这样
// client就能通过这个sockaddr找到server
int bind(int sockfd, const struct sockaddr* addr, socklen_t len);

// 3. 设置socket为一个被动socket，也就是说这个socket用于接收连接
// server调用listen后，会维护一个链表，后续尝试连接此socket的请求
// 会插入到这个链表中
int listen(int sockfd, int backlog);

// 4. 从链表中获取连接请求，创建socket和连接。注意这里的sockaddr
// 和socklen_t都是输出参数，保存了对端的address。accept()会返回
// 一个socket fd用于和对端通信
int accept(int sockfd, struct sockaddr* addr, socklen_t* addrlen);
```

客户端连接的过程：
```C
// 1. 创建socket，上面已经讲过了
int socket(int domain, int type, int protocol);

// 2. 主动去和server之间创建连接。上面我们说了server的listen()会把
// socket变成一个被动的socket，它等待的就是client端的主动connect()。
// 一般情况下客户端不会去绑定address，如果客户端的socket没有绑定
// address，connect()的时候系统会自动绑定。
int connect(int sockfd, const struct sockaddr* addr, socklen_t len);
```

上面服务端和客户端的api中都用到了`struct sockaddr`，这就是一个socket地址定义，因为在网络连接中，客户端必须要知道服务端的地址才能与之连接，sockaddr就是在网络中唯一识别一个机器上某一个socket的标志:
```C
// sockaddr是一个统一的接口，所有的域都使用这个统一的接口，比如AF_INET
// 和AF_INET6，它们bind使用的信息肯定是有差异的，但是为了上层使用的时候
// 接口统一，他们都必须强制转换成sockaddr，正是因为如此，才用
// socklen_t来指定这个结构的实际长度。
struct sockaddr {
    sa_family_t sa_family;
    char sa_data[14];
};

// IPV4中需要指定域、端口和4字节的address。
struct sockaddr_in {
    sa_family_t sin_family;
    in_port_t sin_port;
    struct in_addr sin_addr;
};
struct in_addr {
    uint32_t s_addr;
};

// IPV6中需要指定域，端口，flow信息，16字节的address，以及一个scope_id。
struct sockaddr_in6 {
    sa_family_t sin6_family;
    in_port_t sin6_port;
    uint32_t sin6_flowinfo;
    struct in6_addr sin6_addr;
    uint32_t sin6_scope_id;
};
struct in6_addr {
    unsigned char s6_addr[16];
}
```

数据的发送和接收，当连接建立后，客户端和服务端都有一个fd与对端通信，我们可以把它当成普通的fd使用，这时候直接用`read()`、`write()`即可，为了更加细致的控制数据传输的过程，可以用下面的api：
```C
// 发送数据，有一个flags参数，可以对发送过程进行一些额外的控制。
ssize_t send(int sockfd, const void* buf, size_t nbytes, int flags);

// 面向无连接的socket可以用sendto，当然面向连接的也可以用，只不过
// destaddr和destlen被忽略
// send(fd, buf, len, flags) = sendto(fd, buf, len, flags, NULL, 0);
ssize_t sendto(int sockfd, const void* buf, size_t nbytes, int flags, const struct sockaddr* destaddr, socklen_t destlen);

// sendmsg()一般被用来发短的控制信息。
ssize_t sendmsg(int sockfd, const struct msghdr* msg, int flags);

```

```C
// 接收数据，相比read()多了一个flags参数，控制接收的细节。接收数据的时候
// 需要注意，对于SOCK_STREAM类型的连接，不能认为recv能一次全部接收，必须
// 要循环recv来保证接收了所有数据。所以一般会自定义包结构来区分一个包和
// 另一个包。
ssize_t recv(int sockfd, void* buf, size_t nbytes, int flags);

// 一般用于面向无连接的socket。
// recv(fd, buf, len, flags) = recvfrom(fd, buf, len, flags, NULL, NULL);
ssize_t recvfrom(int sockfd, void* restrict buf, size_t len, int flags, )

// 一般用于接收控制信号。
ssize_t recvmsg(int sockfd, struct msghdr* msg, int flags);
```

有了上面这些API，我们就可以自己写一个客户端和服务端了，代码见[client](./16_3_client.c)和[server](./16_3_server.c)
```shell
# server
./16_3_server 41000 1 5 

# client
./16_3_client 127.0.0.1 41000 uptime
```


# 16.4
这个题目也很简单，要返回服务端上当前运行的进程数量，可以使用`ps -ef | wc -l`，为了满足server的拓展性，我们对server进行改进，让它读取client的命令，然后调用不同的cmd。

client和server直接使用16_3的client和server即可。
```shell
# server
./16_3_server 41000 1 5 

# client
./16_3_client 127.0.0.1 41000 process_count
```

# 16.5
要说明的是，像图16_18这种服务器的处理方式本身就是很低效的，在实际工程中基本上不会用这种方式写server，它存在的问题有这样几个：
1. 每一个请求都需要fork()一个进程进行处理，进程创建和销毁的开销很大。
2. tcp连接的处理和请求的处理没有解耦，是同步进行的，这样导致请求的平均耗时会很长。
3. 在用了多进程的情况下，还需要等待一个子进程完成请求后才能响应下一个请求。

本题作者希望我们能解决它的第3点问题，方法也很简单，父进程利用信号去处理子进程的回收，`waitpid()`写在信号处理函数里，由于这个方案本身问题很多，这里我们就不写代码了。

# 16.6
题目的要求是我们提供一套代码能够启用和禁用socket的异步IO，代码如下：
```C
int enable_socket_async(int sockfd) {
    if (fcntl(sockfd, F_SETOWN, getpid()) < 0) {
        return -1;
    }

    int enable = 1;
    if (ioctl(sockfd, FIOASYNC, &enable) < 0) {
        return -1;
    }
    return 0;
}

int disable_socket_async(int sockfd) {
    int enable = 0;
    if (ioctl(sockfd, FIOASYNC, &enable) < 0) {
        return -1;
    }
    return 0;
}
```