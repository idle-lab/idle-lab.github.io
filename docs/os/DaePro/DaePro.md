

## **基础概念**

- **进程组：**进程组：一组协同工作或关联进程的集合，每个进程组有ID(PGID)。每个进程属于一个进程组，每一个进程组有一个进程组长，该进程组长ID(PID)与进程组ID(PGID)相同。一个信号可以发送给进程组的所有进程、让所有进程终止、暂停或继续运行


- **会话：**会话是一个或多个进程组的集合。当用户登录系统时，系统会为这个用户创建一个新的会话(session)。shell进程(如bash) 作为会话的第一个进程，称为会话领导者进程(session leader)。会话的SID （session ID）：等于会话首进程的PID。会话会分配给用户一个控制终端(只能有一个)，用于处理用户的输入输出。一个会话包括了该登录用户的所有活动。会话中的进程由一个前台进程组和N个后台进程组构成。

## **守护进程**

当用户退出时，会话会结束，相应的会话中的进程组也都会结束。一些服务器上的服务我们是不想让他们退出的，所以我们要让这样的进程成为一个守护进程（精灵进程）。

所谓守护进程（daemon），就是一类在后台运行的特殊进程，用于执行特定的系统任务。很多守护进程在系统引导的时候启动，并且一直运行直到系统关闭。另一些只在需要的时候才启动，完成任务后就自动结束。

### **创建守护进程**

我们通过系统调用 `setsid` 来创建一个新会话，并且将调用进程的进程 PID 设为这个新会话的 SID。为了保证会话的唯一性，创建新会话的进程不能是一个 leader 进程或进程组组长。

```cpp
#include <sys/types.h>
#include <unistd.h>

pid_t setsid(void);
```

通常我们自己创建一个守护进程，就需要通过 fork 子进程的方式，因为新 fork 出的子进程一定不是一个 leader 进程或是进程组组长，我们再把父进程退出，让子进程执行服务器代码。所以守护者进程事实上就是一个孤儿进程。通常情况下要成为守护进程的程序命名时都会以 d 做结尾，如：sshd、mysqld等。

??? note "Craete daemon processs"
    ```cpp
    #include <sys/types.h>
    #include <unistd.h>
    #include <signal.h>
    #include <unistd.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <stdlib.h>

    void daemon()
    {
        // 1. 忽略异常信号，保证守护进程不轻易退出
        signal(SIGPIPE,SIG_IGN);
        signal(SIGCHLD,SIG_IGN);
        // .... 按照需求忽略其他信号

        // 2. fork 创建子进程
        if(fork() > 0) exit(0); // 退出父进程，子进程变为孤儿进程

        // 3. 创建新会话
        setsid();

        // 4. 按照需求切换守护进程工作目录
        chdir("/");

        // 5. 处理 stdin、stdout、stderr
        // 使用 /dev/null，向该文件写入的所有数据都会被丢弃
        // 也可以直接关闭三个输入输出流，看项目需求
        int fd = open("/dev/null",O_RDWR);
        dup2(fd,0);
        dup2(fd,1);
        dup2(fd,2);
    }

    int main()
    {
        daemon();
        // 此时的进程已经成为一个守护者进程
        // 所有后续代码都是在后台执行

        for(;;)pause();

        return 0;
    }
    ```

除了自己手写，系统也为我们提供了，创建守护进程的接口：

```cpp
#include <unistd.h>

int daemon(int nochdir, int noclose);
```

- nochdir：为 0 时，修改当前进程的工作目录为根目录，否则不做修改。

- noclose：为 0 时。修改当前进程的 stdin、stdout、strerr 为 /dev/null ，否则不做修改。

成功返回 0 ，失败返回 -1。并且该函数时线程安全的。