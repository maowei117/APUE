# 13.1
因为`openlog()`在底层是默认打开`/dev/log`这个socket文件的，所以如果我们在`openlog()`或者`syslog()`之前先调用了`chroot()`，那么我们会找不到`/dev/log`这个socket文件，这会导致日志的打印失败。

但是有时候我们会有这样的需求：在用户程序中需要`chroot()`，又需要打印daemon的日志，这时候我们需要在`chroot()`之前先打开`/dev/log`这个文件，生成对应的fd，然后再`chroot()`，已经生成好的fd是不受`chroot()`影响的。

# 13.2
这个题目我去我的机器上试了一下，发现`rsyslogd`在我的机器上已经是会话的首进程了：
```shell
ps -axj  | grep rsyslogd
1  6355  6355  6355 ?           -1 Ssl      0  13:41 /usr/sbin/rsyslogd -n
```

然后我以为是有什么和机器相关的小trick，结果查了一下才发现，这是`rsyslogd`早期在实现时候的一个bug，这里有一个[帖子](https://unix.stackexchange.com/questions/333512/rsyslogd-as-a-session-leader)解释得很好。

这其实源于以前`rsyslogd()`实现中的一个bug，这个bug导致其中的`setsid()`没有被调用，所以`rsyslogd()`的sid才和pid不相同，不是session leader。

这里我们回顾一个重要的问题：为什么一个daemon进程不能打开控制终端呢？
因为daemon进程一般来说是要常驻在后台运行的，而当我们关闭控制终端的时候，会给对应的进程发送一个`SIGHUP`的信号，这个信号通常会使得进程终止。

这就是为什么我们通过终端软件登录到远程开发机上并运行进程时，如果我们关闭了登录的终端，我们运行的进程也会随之终止。

所以当我们开发一个daemon进程的时候，我们需要通过`setsid()`保证这个daemon进程脱离所有的控制终端。

其实不调用`setsid()`也是可以的，只要我们通过`ioctl()`将所有打开的fd全部都设置`O_NOCTTY`这个flag就可以了。

# 13.3
我们就挑选几个讲一下好了：
```
ps -axj
```
1. cron 用于定时执行命令，我们一般用crontab命令来编辑要执行的定时命令，这个daemon经常被用于服务的状态check，然后自动拉起。
2. syslogd 系统记录日志，其他的daemon利用这个daemon去收集日志。
3. sshd 用于管理ssh会话。
4. dockerd 这个是docker用于管理的daemon，是一个应用级别的daemon。

# 13.4
代码见[13_4](./13_4.c)，当进程已经daemonize之后，再调用`getlogin()`会发现结果为null，因为daemon是没有控制终端的。

回顾图9-7我们能发现几个重点：
1. 控制终端只连接到会话首进程，当控制终端被关闭，SIGHUP信号会发送到这个进程。
2. 终端的输入和输出产生的信号只会发送到前台进程组，一个会话中只有一个前台进程组。
3. 后台进程组不会接收任何来自终端的信号，所以终端即使断开，后台进程组也不会因此退出，适合作为daemon进程。

这里我们对daemon再次进行一下回顾，一个普通的进程如何成为一个daemon进程呢？
1. 脱离控制终端，一般通过`fork()` + `setsid()`来脱离当前的控制终端，其中`fork()`保证了当前的进程不是进程组的组长，然后调用`setsid()`就一定能创建一个新的会话，从而才能利用`setsid()`切换控制终端。
2. 保证不要打开tty终端，有很多措施是为了保证这一点。例如在`setsid()`后再进行一次`fork()`，通过这种方式保证进程不是session leader，不会打开控制终端。另外还有利用`O_NOCTTY`打开daemon中的文件，这样就确保不会打开tty终端。这个例子中还`close()`了0 - RLIM_INFINITY的fd，并且将stdin/stdout/stderr重定向到/dev/null，这些措施都是为了让守护进程切断和tty终端的联系。
