# 19.1
以telnet为例，整体流程如下：
1. telnet客户端创建socket，`connect()`发起连接，telnet服务端`listen()`收到连接请求，`accept()`接收连接，至此网络连接建立。
2. telnet服务端识别到这是一个新的连接，于是需要为这个新的连接创建一个伪终端：`poxsix_openpt()` + `grantpt()` + `unlockpt()` + `ptsname()`得到对应的master fd和slave file name。
3. 得到了伪终端的master和slave之后，telnet服务端`fork()`创建子进程，利用:
```C
dup2(slave_fd, STDIN_FILENO);
dup2(slave_fd, STDOUT_FILENO);
dup2(slave_fd, STDERR_FILENO);
``` 
将子进程的标准输入，输出，错误重定向到伪终端的slave file。
4. 重定向完成后，子进程调用`exec()`执行login程序，完成登录。
5. 登录完成后，子进程调用`exec()`执行shell程序。
6. 正常运行时，数据的流向为socket-> master fd -> slave file -> shell -> process -> shell -> slave file -> master fd -> socket。

在这个过程中pty slave设备是在第二步创建的，在`grantpt()`时就会把user id设置为登录的id，并把group id设置为tty。

# 19.2
题目的要求很简单，我们只需要实现书中的pty程序，然后调用`pty -n stty -a`就可以显示系统初始化时的termios和winsize结构了。

为了简便起见，我们就不写跨平台相关的代码了，代码见[pty](./pty.c)

在我的系统上，初始化的termios和winsize均为0.

# 19.3
loop()函数里面子进程从stdin读，写入到ptym中，然后父进程从ptym中读，写入到stdout中。通过使用poll()，我们可以在单进程中完成IO多路复用，代码如下：
```C
void loop(int ptym, int ignoreeof) {
    int num = 2;
    struct pollfd pfds[num];
    pfds[0].fd = ptym;
    pfds[0].events = POLLIN;
    pfds[1].fd = STDIN_FILENO;
    pfds[1].events = POLLIN;
    int write_fds[num];
    write_fds[0] = STDOUT_FILENO;
    write_fds[1] = ptym;

    const int bufsize = 4096;
    char buffer[bufsize];

    while (1) {
        int n = poll(pfds, num, -1);
        if (n < 0) { break; }

        for (int i = 0; i < num; ++i) {
            if (pfds[i].revents & POLLIN) {
                // Read.
                int rfd = pfds[i].fd;
                int nread = read(rfd, buffer, bufsize);
                if (nread <= 0) { return; }

                int wfd = write_fds[i];
                if (write(wfd, buffer, nread) != nread) { return; }
                pfds[i].revents = 0;
            } else if (pfds[i].revents & POLLHUP) { return; }
}
}
}
```

# 19.4
题目中希望能够改变标准输入、输出和错误的读写模式，对这种已经打开的fd，修改其文件状态，我们一般使用`fcntl()`，但是通过`man fcntl`我们可以看到下面这段话：

> Set  the  file  status  flags  to  the  value specified by arg.
>
> File access mode (O_RDONLY, O_WRONLY, O_RDWR) and file creation flags (i.e., O_CREAT, O_EXCL, O_NOCTTY, O_TRUNC) in  arg are ignored.
>
> On Linux, this command can change only the O_APPEND, O_ASYNC, O_DIRECT, O_NOA‐ TIME, and O_NONBLOCK flags.
>
>It is not possible to change the O_DSYNC and O_SYNC flags.

这表明，文件的访问模式是不能通过fcntl进行修改的。

# 19.5
图19-13画得已经很清楚了：

+ 登录shell是后台进程组，因为它没有连接到控制终端。
+ pty进程组和cat进程组都是前台进程组，pty进程组连接到了pty的master端，cat连接到了pty的slave端。

# 19.6
收到EOF时，流程如下：
1. cat进程终止，关闭pty slave fd.
2. pty父进程中pty master read到了EOF，于是发送SIGTERM信号给pty子进程，子进程终止
3. 父进程等待子进程退出后，exit(0)退出。

# 19.7
通过`date`命令就可以知道当前的时间了。
```shell
(echo "Start:`date`"; pty "${SHELL:-/bin/sh}"; echo "Finish:`date`") | tee typescript
```

# 19.8
很显然这里多出来的两行打印是data文件的内容。对比图19-13我们可以发现，pty在读入标准输入后，会经过终端行规程，所以这个输出应该是终端行规程的回显。

# 19.9
按照题目要求实现即可，代码见[19_9](./19_9.c)