# 2.1
这个问题是在说，如果A.h和B.h两个头文件中都typedef了同一个类型，那么当C.c引用A.h和B.h的时候，编译器就会报错，因为相当于在同一个编译单元中出现了一样的类型。

解决这种编译重复的问题可以用条件编译：
```
#ifndef SIZE_T_DECLARED_
typedef long size_t
#define SIZE_T_DECLARED_
#endif 
```
这样编译器会选择先#include的那个头文件来定义size_t。

# 2.2
我们指导系统数据类型定义是在sys/types.h，可以通过find命令找到具体的位置
```
find / -name "types.h" | grep "sys/types.h"
```
我的系统是ubuntu20.04，在我的机器上这个头文件在`/usr/include/x86_64-linux-gnu/sys/types.h`
我就不一个一个列举了，我用一个pid_t作为例子来找一下吧，首先是这个sys/types.h中是这样定义的：
```
#ifndef __pid_t_defined                      
typedef __pid_t pid_t;
# define __pid_t_defined
#endif
```
这里可以看到2.1题中我们的说法是正确的，确实是用了预编译指令来包裹typedef，不过这里我们发现`pid_t`并不是直接定义的，而是一种内部类型`__pid_t`，我们发现在`sys/types.h`中包含了`bits/types.h`，所以我们去`bits_types.h`中看看`__pid_t`是什么:
```
#define __STD_TYPE typedef 
__STD_TYPE __PID_T_TYPE __pid_t
```
这里又故伎重施，再找一下这个`__PID_T_TYPE`是个啥,在`bits/types.h`里面我发现了这样一个注释：
```
/* The machine-dependent file <bits/typesizes.h> defines __*_T_TYPE
 ... 
*/
```
原来是在`bits/typesizes.h`里面，我们到里面终于找到了这个`__PID_T_TYPE`，再结合`bits/types.h`里面的基本的一些数据类型，我们终于知道了：**在我的机器上，pid_t是一个int**。
```
#define __S32_TYPE int
#define __PID_T_TYPE __S32_TYPE
```
这个过程是比较枯燥无趣的，但是我们可以思考一下为什么操作系统要把系统的基础类型定义搞得这么复杂，而不是简单的一个typedef在`sys/types.h`里面就完事儿。
我们还是利用pid_t这个例子来说，其中涉及了三个头文件：
```
sys/types.h
bits/types.h
bits/typesizes.h
```
1，首先我们解释为什么不直接在`sys/types.h`中去定义`typedef int pid_t`，因为`sys/types.h`是标准库的头文件，这个头文件是会暴露给用户使用的，如果我们直接在这个头文件中去暴露了`pid_t`是一个`int`，那么就相当于给用户暴露了底层的实现，这样用户在需要`pid_t`的时候可能就用一个`int`直接代替了，从而失去了移植性。所以不在`sys/types.h`中直接定义的原因，我认为是为了封装，隐藏底层的实现。

2，既然是为了隐藏底层实现，那么为什么在`bits/types.h`中还要再typedef一次，使用`bits/typesizes.h`里面的定义呢？这里，我找到了`bit/typesizes.h`源码中的一句注释：
```
/* See <bits/types.h> for the meaning of these macros.  This file exists so that <bits/types.h> need not vary across different GNU platforms.*/
```
原来在bits/types.h中再使用一次typedef是出于跨平台的考虑，比如说一套Linux的C系统库，我需要编译32位版本的和64位版本的，通过将平台的差异放在`bits/typesizes.h`中，这样就能保证`bits/types.h`是可以一直保持不变的，这样我们可以一次编译就编译出32位版本和64位版本，而不需要维护32位平台和64位平台各自的`bits/types.h`。

其实这也是Don't Reapet Yourself的一个体现，在`bits/types.h`中维护不同平台中相同的部分，把真正差异的部分放在`bits/typesizes.h`中。

# 2.3
这个问题的难点主要在于不知道中文版的翻译到底想让你干什么，其实这个问题需要结合正文的部分来理解，是这样的，有些程序经常会获取“单个进程同时打开文件的最大值”，然后他们会这样来用：
```
for (int i = 0; i < open_max; i++) {
    close(i);
}
```
这样就可以关闭掉所有已打开文件了。但是有些系统它的实现把`sysconf(_SC_OPEN_MAX)`的返回值实现为LONG_MAX，这就导致我需要调用close()很多次，其实打开的文件并不会有LONG_MAX这么多个，所以就导致了很多无用的循环，那么如何能够减少这些不必要的循环的次数呢？

这个问题其实作者已经给出了答案，就是`getrlimit()`，这个调用是一个拓展调用，意思就是"get resource limits"，它的api是这样的：
```
int getrlimit(int resource, struct rlimit* rlim);

struct rlimit {
    rlim_t rlim_cur;  /* Soft limit */
    rlim_t rlim_max;  /* Hard limit (celing for rlim_cur)*/
}
```
所以我们只需要在`sysconf(_SC_OPEN_MAX)`返回为LONG_MAX的时候使用`get_rlimit()`的值就可以了，代码见[2_3](./2_3.c)




