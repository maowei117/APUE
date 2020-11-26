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
