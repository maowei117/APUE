# 前言
本章的写作逻辑其实很简单，就是从`stat()`这个函数出发，引出UNIX系统中文件的各种属性，进而引出针对每个属性的操作。学习本章主要就是要搞清楚UNIX文件系统的组成方式。

# 4.1
图4.3的程序其实很简单，就是把命令行参数都认为是一个文件的path，然后直观地打印出对应的文件是哪种类型的文件，我们可以做一个对比的程序，把stat和lstat的结果都打印出来，代码见：[4_1](./4_1.c)
尝试之后可以发现，其实`stat()`和`lstat()`之间的区别主要在于`stat()`会follow symbolic，也就是说，当打开的文件是一个软链接的时候，stat得到的信息实际上是软链接指向的文件，而`lstat()`则不会这样，当打开的文件是一个软链接的时候，`lstat()`得到的是这个软链接文件本身的信息，所以得到的文件类型也是symbolic link，知道了这个行为，我们猜测，这个`lstat()`中的l，就是link的意思，用于获取link的stat。

到这里这道题就算是结束了，但是我们还可以看一下`sturct stat`中`st_mode`是怎么组织的。这个`st_mode`一个字段可以表示7种文件类型，其实就是用了位运算，在我所在的系统中，位运算是这样做的：
```
#define S_IFMT          0170000         /* [XSI] type of file mask */
#define S_IFIFO         0010000         /* [XSI] named pipe (fifo) */
#define S_IFCHR         0020000         /* [XSI] character special */
#define S_IFDIR         0040000         /* [XSI] directory */
#define S_IFBLK         0060000         /* [XSI] block special */
#define S_IFREG         0100000         /* [XSI] regular */
#define S_IFLNK         0120000         /* [XSI] symbolic link */
#define S_IFSOCK        0140000         /* [XSI] socket */

#define S_ISBLK(m)      (((m) & S_IFMT) == S_IFBLK)     /* block special */
#define S_ISCHR(m)      (((m) & S_IFMT) == S_IFCHR)     /* char special */
#define S_ISDIR(m)      (((m) & S_IFMT) == S_IFDIR)     /* directory */
#define S_ISFIFO(m)     (((m) & S_IFMT) == S_IFIFO)     /* fifo or socket */
#define S_ISREG(m)      (((m) & S_IFMT) == S_IFREG)     /* regular file */
#define S_ISLNK(m)      (((m) & S_IFMT) == S_IFLNK)     /* symbolic link */
#define S_ISSOCK(m)     (((m) & S_IFMT) == S_IFSOCK)    /* socket */
```
是8进制的位运算，实际上只用了4位就表示了这些文件类型。其实UNIX系统中很多用到了这种技巧，当想要节约空间的时候，一个int字段，可能会表示多种类型的信息，比如说其中的几个bit想要表示mode，另外一些bit想要表示link size，就可以不暴露底层的实现，而是通过定义一些宏的方式来操作这个字段。对于现在的硬件来说，这点空间可能已经无足轻重了，但是在很多嵌入式环境里面，这种技巧用得很多。


