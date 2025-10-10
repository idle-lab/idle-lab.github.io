
## **概念**

### **为什么要有线程?**

一个程序可能要同时处理多个并发任务，我们可以针对一个程序，我们可以创建多个子进程，来分别完成不同的任务。但是进程的问题是太 “重” 了。

- 进程创建很 “重”： 对于操作系统创建一个进程的代价很高，要加载可执行文件，创建进程地址空间及页表映射，还要加载需要的动态库...
- 进程切换很 “重”： CPU 切换执行进程时，CPU 中寄存器的数据都要切换，更主要的是进程切换后 Cache 上的数据大部分都会失效，要重新从内存加载。

所以我们需要一种更轻量级的执行流，来处理这样的并发任务。

当我们创建了一个进程后，我们可以为其创建多个执行流，每个执行流就是一个线程，每个线程执行程序的一部分，线程间共享进程的地址空间，文件描述符表，代码及全区变量等等资源，但不同的线程间有独立的栈和硬件上下文。**我们可以说线程是 CPU 调度的基本单位，而进程是承担系统资源的基本实体。**

线程的操作并为涉及系统资源的分配，只是进程资源的再分配，所以线程的创建、处理、调度整个都是在用户态进行的。

### **Linux 多线程的实现**

在 Linux 下，多线程的实现复用了进程的代码，采用了一种轻量级进程（Light Wetght Process,LWP）的方式实现。对于轻量级进程，创建它只需要创建 `task_struct` ，多个轻量级进程间会共享进程的一些资源，这与线程是类似的。


## **线程控制**

### **创建**

使用 `pthread_create` 创建一个线程。

```cpp
#include <pthread.h>

int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                    void *(*start_routine) (void *), void *arg);
```

pthread_create 函数的参数列表中， thread 是输出型参数，返回线程 id。 attr 是线程属性，一般设置为nullptr。函数指针用来回调式的执行目标函数。 arg 是传递给回调函数的参数。线程创建成功返回 0 ，失败返回 -1 ，并且对全局变量 errno 赋值以指示错误。

这里我们创建线程得到的线程 id 就是线程属性集的地址，方便我们快速通过线程 id 来定位线程。

我们也可以通过调用以下函数获取当前线程的线程 id。

```cpp
#include <pthread.h>
pthread_t pthread_self(void);
```

??? code "线程创建"
    ```cpp
    #include <iostream>
    #include <pthread.h>
    #include <unistd.h>
    #include <tuple>
    using namespace std;

    // 交给 线程1 执行的代码
    void* pthread1(void* arg)
    {
        // 获取传入的参数
        auto&& [a,b,c] = *static_cast<tuple<int,double,char>*>(arg);
        for(;;)
        {
            cout << "I am thread1:" << a << ' ' << b << ' ' << c << endl;
            sleep(1);
        }
    }

    int main()
    {
        pthread_t tid = 0;
        
        // 使用 tuple 传递参数
        tuple<int,double,char> td{1,1.2,'a'};

        // 创建线程
        pthread_create(&tid,nullptr,pthread1,static_cast<void*>(&td));

        // 主线程
        for(;;)
        {
            cout << "I am thread0:" << endl;
            sleep(1);
        }
        return 0;
    }
    ```

### **退出**

直接在线程执行函数中 `return` ，但是对于主线程直接 `return` 会导致整个进程的终止，`exit` 同理。

使用 `pthread_exit` 可以只终止自己。

```cpp
#include <pthread.h>
void pthread_exit(void *retval);
```

retval 时当前线程结束时的返回值。

可以通过 `pthread_cancel` 可以选择关闭指定线程。

```cpp
#include <pthread.h>
int pthread_cancel(pthread_t thread);
```

### **等待**

线程结束后，需要有线程等待该线程，将该线程回收，并获取返回值。

```cpp
#include <pthread.h>

int pthread_join(pthread_t thread, void **retval);
```

thread 等待的线程 id ，retval 输出型参数，返回等待线程的返回值，不需要时置为 nullptr。等待方式为阻塞式等待。

等待被 cancel 的线程时，得到的返回值是一个宏 `PTHREAD_CANCELED` ，其值为 -1。


??? code "线程控制"
    ```cpp
    /*
    实验结果：两秒后，旧主线程退出，被新的主线程回收。
    */
    #include <iostream>
    #include <pthread.h>
    #include <unistd.h>
    #include <tuple>
    #include <vector>
    #include <string>
    using namespace std;

    vector<pthread_t> tids; 

    string to_hex(long tid)
    {
        char buff[512];
        snprintf(buff,sizeof buff,"0x%lX",tid);
        return buff;
    }

    void* real_main(void* arg)
    {
        // 获取传入的参数
        auto&& [argc,argv,env] = *static_cast<tuple<int,char**,char**>*>(arg);

        char* main_ret = new char[512];
        // 回收退出的原 mian 函数，并打印原主线程的返回值
        pthread_join(tids[0],(void**)&main_ret);
        cout << "Old main return :" << main_ret << endl;
        for(;;)
        {
            cout << "I am New main thread:" << to_hex(pthread_self()) << endl;
            sleep(1);
        }
    }

    int main(int argc,char* argv[],char* env[])
    {
        // 记录主线程的 线程id
        tids.push_back(pthread_self());

        pthread_t tid = 0;
        tuple<int,char**,char**> td{1,argv,env};

        // 创建线程
        pthread_create(&tid,nullptr,real_main,static_cast<void*>(&td));
        tids.push_back(tid);
        cout << "Old main thread id : " << to_hex(tids[0]) << endl;
        cout << "New main thread id : " << to_hex(tids[1]) << endl;

        sleep(2);
        // 主线程退出
        // 返回值不能为将亡值，否则当栈释放后，会导致指针悬垂。
        pthread_exit((void*)"You are thr real main func!\n");
    }
    ```

### **分离**

当我们不关心一些线程的执行情况时，并且想让其在结束后自动被回收，就可以使用 `pthread_detach` 函数。

```cpp
#include <pthread.h>
int pthread_detach(pthread_t thread);
```

thread 是要分离的线程 id。可以是线程组内其他线程对目标线程进行分离，也可以是线程自己分离。

当一个线程被设为分离状态时，其他线程是不能对其进行 join 的。

### **局部存储**

不同线程间会共享进程的很多资源，但是线程还有独立的数据来执行不同的任务，如：栈，硬件上下文...。

对于不同的线程全局变量是共享的，各个线程都可以看到，但是我们可以用过编译器的一些标志为将全局变量声明为线程的局部存储。典型的就是errno全局变量，它总是会保存当前线程最后一个调用的错误码，不会存在线程冲突。

??? code "__thread"
    ```cpp
    /*
    结果：只有 g_val 在两个线程中都会改变，thread_g_val 只在修改的线程中改变
    */
    #include <iostream>
    #include <pthread.h>
    #include <fstream>
    #include <unistd.h>
    #include <string>
    using namespace std;

    // 线程 id 转 16 进制
    string to_hex(long tid)
    {
        char buff[512];
        snprintf(buff,sizeof buff,"0x%lX",tid);
        return buff;
    }

    int g_val = 0;                  // 常规全局变量
    __thread int thread_g_val = 0;  // 声明线程局部存储
    // __thread string thread_name; 只能作用于内置类型

    void* headle(void* arg)
    {
        for(;;)
        {
            cout << "Other Thread:" << to_hex(pthread_self()) << " "
                << "g_val:" << g_val << "; " << "thread_g_val:" << thread_g_val << endl;
            
            // 在其他线程中修改全局变量
            g_val++;
            thread_g_val++;
            sleep(1); 
        }
    }

    int main(int argc,char* argv[],char* env[])
    {
        pthread_t tid;
        pthread_create(&tid,nullptr,headle,nullptr);

        for(;;)
        {
            cout << "Main Thread:" << to_hex(pthread_self()) << " "
                << "g_val: " << g_val << ";  " << "thread_g_val:" << thread_g_val << endl;
            sleep(1);
        }

        return 0;
    }
    ```

## **线程库**

### **Linux 原生线程库**

Linux 原生线程库通常使用 POSIX 兼容的 pthread 库。pthread 库是对系统调用的封装，来对创建出的 LWP 做类似线程的管理。

在 `pthread_create` 函数就是对系统调用 `clone` 的封装，同时他也是 `fork` 函数的低层。

```cpp
// clone, __clone2, clone3 - create a child process

#define _GNU_SOURCE
#include <sched.h>

int clone(int (*fn)(void *), void *stack, int flags, void *arg, ...
            /* pid_t *parent_tid, void *tls, pid_t *child_tid */ );
```

在调用 pthread_create 函数时，会把回调函数的指针传递给系统调用 clone ，并且在库中创建新线程的结构体，并把线程栈地址传给clone的参数 child_stack 。在调用不同的线程时，通过更新寄存器 esp、ebp 的值就可以实现栈的切换，因此就算多个线程共同调用同一个函数，因为线程栈不同，所创建的临时变量的地址也不同。

### **C++线程库**

在C++11之前，涉及到多线程问题，都是和平台相关的，比如 windows 和 linux 下各有自己的接口，这使得代码的可移植性比较差。C++ 11 标准以后对线程进行了支持，使得C++在并行编程时不需要依赖第三方库，而且在原子操作中还引入了原子类的概念。

在头文件 `<thread>` 中，提供了 [`thread`](https://legacy.cplusplus.com/reference/thread/thread/){target="_blank"} 类。




