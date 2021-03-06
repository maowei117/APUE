# 5.1
通过这个题目, 我们对stdio的缓冲区进行一下总结. 什么叫做缓冲呢, 以前的磁盘大多是机械磁盘, 每次写磁盘比较耗时(涉及到磁盘内部的机械结构的运动, 写的次数多了磁盘也会有损耗). 为了减轻磁盘的负担, 操作系统会先在内存中申请一块buffer, 写入的时候先往这个buffer写, 等到这个buffer满了才把数据真正写入磁盘中, 这种方式就叫做缓冲.
其实stdio的缓冲方式有三种:
1. 全缓冲, 只有当buffer被写满或者调用`fflush()`的时候才会把内存buffer中的数据写入磁盘. 一般情况下写文件默认采用的是全缓冲.
2. 行缓冲, 当buffer中写入了`\n`, 或者写满, 调用`fflush()`的时候把数据写入磁盘, 一般情况下是交互类型的文件, 比如说终端tty文件, 采用这种方式. 
3. 不缓冲, 这种方式就和直接调用`write()`差不多, 一般用于需要马上响应的数据, 比如`stderr`的输出.

`setbuf`和`setvbuf`就是用来修改默认的缓冲行为的函数, 题目中要求我们用setvbuf实现setbuf, 其实就是下面的逻辑:
```C++

void setbuf(FILE* restrict fp, char* restrict buf) {
    if (buf == NULL) {
        setvbuf(fp, NULL, _IONBF, 0);
    } else {

        setvbuf(fp, buf, _IOFBF, BUFSIZ);
    }
}

```

# 5.2
代码见[5_2](./5_2.c), 这个问题是希望我们能够理解`fgets()`和`fputs()`的用法.

首先fgets()从文件中读时, 函数正常的返回情况有这样几种:
1, 读到'\n'或者'\0'
2, 读入的长度 + 1 >= buffer_len. 这里的 +1就是为最后的'\0'预留了位置
3, 读到了eof

然后fputs()向文件中写入时, 函数如果正常返回, 一定会把buffer中的所有数据全部写入流.

所以这个题目的结果就很明显了, 当buffer长度比较小的时候, 超过了缓冲区, 那么就是一个流需要被拷贝更多次, 其中的文本内容会保留.
所以结果就是, buffer长度小导致性能降低, 但是功能依然是正确的, 实验也验证了我们的结论. 

# 5.3
这个问题很简单, 我们查一下man page就能知道, printf的返回值表示打印的字符数, 所以printf返回0就表示打印的字符数为0. 但是要注意这个值是没有把末尾的'\0'计算在内的, 所以printf返回0, 执行`printf("");`就可以返回0了.

# 5.4
C标准库经常喜欢把一个返回值赋予多种用途, 例如这个`getchar()`, 当返回值 >= 0的时候, 表示的是get到的char, 但是当返回值 == EOF(一个负值)的时候, 却表示此次getchar的状态. 正是因为这个原因, `getchar()`的返回值是一个int, 而不是char.
char表示字符类型, 在有些系统上实现为unsigned char, 有些又是signed char, 很多系统把EOF实现为-1, 所以说当特定的操作系统把char实现为signed char的时候, 运行可以正确执行, 实现为unsigned char, 就不能正确执行了. 

# 5.5 
我们需要先知道fsync()的作用, 虽然write()和read()是系统调用, 但是实际上内核在处理的过程中, 是可能对数据进行缓冲的. fsync()就是冲洗内核的缓冲区. 对于标准IO, 其实就是对文件描述符fd的一种封装, 有了一个用户级别的缓冲区. 我们想要对标准IO进行fsync(), 需要先flush用户缓冲区, 然后再得到对应的fd, 进行fsync(), 代码比较简单:
```C++
    // Assume FILE* file opend.

    fflush(file)
    int fd = fileno(file);
    fsync(fd);
```

# 5.6
这个题目涉及到标准io一个缓冲的细节, 这个在书中5.4节有提到:

> 任何时候只要通过标准I/O库要求从(a)一个不带缓冲的流 (b)一个行缓冲的流 得到输入数据, 那么就会冲洗所有行缓冲输出流.

对于交互设备, stdin和stdout都是行缓冲的, 在图1.7里面的代码中, 用了fgets(), 这就符合上面的这个条件, 从行缓冲的流中得到了输入数据, 这时候就会flush所有行缓冲的输出流.

# 5.7
由于我没有BSD的环境, 此题就略过了, 在原书后面的答案有源码.  

