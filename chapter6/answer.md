# 6.1
做这个题目之前我们先分析一下为什么要分离/etc/passwd和/etc/shadow
其实这个设计是依赖UNIX的文件权限进行的拆分, /etc/passwd中保存了当前注册的所有用户的信息, 这些信息其实并不是敏感信息, 所以应该是所有人都能够访问的, 但是用户的密码又是属于敏感信息, 应该只有管理员才能看到这些信息. 所以将/etc/shadow这个文件的权限设置为rw-r-----, 所有者为root, 这样只有root用户才能写这个文件. 而/etc/passwd的权限为rw-r--r--, 所有的用户都能读这个文件从而能够知道当前系统的用户信息.

这个题目本身比较简单, shadow文件里面的加密口令, 只要通过root用户打开/etc/shadow文件就可以了, 注意这个保存的不是密码的原文, 而是通过某种单向加密算法得到的密文.

# 6.2
看了半天才明白这道题和上一道题的区别是什么, 上面是希望我们访问/etc/shadow文件得到加密后的密码, 本题是希望我们用程序来得到加密后的密码.
其实很简单, 可以使用`getspnam()`函数, 示例代码见[6_2](./6_2.c)

# 6.3
通过本题我们可以看一下`uname()`函数和`uname(1)`这个程序的对应关系.
首先是`uname()`函数的打印结果, 代码见[6_3](./6_3.c), 结果如下:
```
sysname:Linux
nodename:tommao-HP
release:5.4.0-42-generic
version:#46-Ubuntu SMP Fri Jul 10 00:24:02 UTC 2020
machine:x86_64
domainname:(none)

```

然后我们执行`uname -a`得到结果如下:
```
Linux tommao-HP 5.4.0-42-generic #46-Ubuntu SMP Fri Jul 10 00:24:02 UTC 2020 x86_64 x86_64 x86_64 GNU/Linux
```

通过`man uname`我们可以看到, `uname -a`打印的顺序:
```
       Print certain system information.  With no OPTION, same as -s.

       -a, --all
              print all information, in the following order, except omit -p and -i if unknown:

       -s, --kernel-name
              print the kernel name

       -n, --nodename
              print the network node hostname

       -r, --kernel-release
              print the kernel release

       -v, --kernel-version
              print the kernel version

       -m, --machine
              print the machine hardware name

       -p, --processor
              print the processor type (non-portable)

       -i, --hardware-platform
              print the hardware platform (non-portable)

       -o, --operating-system
              print the operating system
```
其中最后的processor type和hardware-platform是我这个版本的Linux系统提供的, 标明了non-portable. 可以看到, uname(1)这个程序的信息基本上都来自于系统调用`uname()`

# 6.4
time_t记录的是从公元1970年1月1日00:00:00以来经过的秒数, 题目应该是想让我们算出time_t能表示的最大的时间是多少. 很明显这取决于time_t这个字段的实现. 我们这里就用程序来实现一下, 见[6_4](./6_4.c).
最后我们得到结果, 如果是32位的time_t的话, 可以表示到2038-01-19 03:14:07, 这一点我们在`man 2 time`中也能够得到说明.

# 6.5
这个题目很简单, 主要考察`strftime()`中格式化字符串的一些使用, 代码见[6_5](./6_5.c)
在我的机器上, date结果如下:
```
2020年 12月 12日 星期六 22:20:36 CST
```
程序结果如下:
```
# TZ为空
2020-12-12 Saturday 14:21:35 UTC

# TZ=Asia/Shanghai
2020-12-12 Saturday 22:20:55 CST

```
