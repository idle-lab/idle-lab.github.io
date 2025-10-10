
## **wrk**

一款简单的 HTTP 压测工具，它能够在单机多核 CPU 的条件下，使用系统自带的高性能 I/O 机制，通过多线程和事件模式，对目标机器产生大量的负载。

```shell
wrk [选项] [url]
```

常用选项：

- -c, --connections：保持的连接个数

- -d, --duration：持续时间

- -t, --threads：线程数


## **xargs**

xargs 可以将管道或标准输入（stdin）数据转换成命令行参数，也能够从文件的输出中读取数据。
参数间是以空格分隔的，xargs 默认会将所有参数都直接给需要执行的命令。

```shell
[command] | xargs [选项] [command]
```

常用的属性如下：

- -a file 从文件中读入作为 stdin。
  
- -n num 后面加次数，表示命令在执行的时候一次用的argument的个数，默认是用所有的。
  
- -d delim 分隔符，默认的 xargs 是以空格和换行为分隔符，这里会修改为空格和指定的分隔符。

- -i 或者是-I，这得看linux支持了，将xargs的每项名称，一般是一行一行赋值给 {}，可以用 {} 代替。

- -t 表示先打印命令，然后再执行。

下面是一些例子：

```shell
# t 文件下有如下内容：
a b c
d e f
# 例子
$ cat t | xargs
a b c d e f
$ cat t | xargs -n 2
a b
c d
e f
$ cat t | xargs -I {} echo -p {} -l
-p a b c -l
-p d e f -l
$ cat t | xargs -t -I {} echo -p {} -l
echo -p 'a b c' -l
-p a b c -l
echo -p 'd e f' -l
-p d e f -l
$ cat t | xargs -d b
a   c
d e f
```


## **netstat**

打印网络连接、路由表、接口统计信息、伪装连接和多播成员身份。

常见选项：

```shell
netstat [address_family_options] [--tcp|-t]  [--udp|-u] [--numeric|-n]
netstat {--interfaces|-i} [--all|-a] [--program|-p] 
```

- -p ：显示服务名称。

- -l ：显示 listen 状态的服务，不带就显示非 listen 状态的服务。 

- -a ：显示所有状态的服务。


eg:`netstat -aup` 显示当前系统中所有 udp 协议的进程。

还有一个 `ss` 命令和 `netstat` 命令效果相同。

## **pidof**

在查看服务器的进程id时非常方便.

```shell
pidof [进程名] 
```

功能：通过进程名, 查看进程id


## **jobs/fg/bg**

查看当前用户启动的后台进程及其编号和状态。

我们可以运行可执行文件的指令后加上 `&` 来直接后台启动。

```cpp
[可执行程序] &
```

我们可以使用 `fg` 指令将一个后台进程变为前台进程。

我们可以使用 `ctrl + z` 将一个前台进程暂停挂起到后台。使用 `bg` 命令可以将暂停的进程重新运行。

```cpp
fg [任务编号]
bg [任务编号]
```

## **ipcs/ipcrm**

ipcs 显示 System V IPC 的各种设施：共享内存，消息队列，信号量组。
有以下选项：
```shell
-m, --shmems
    Write information about active shared memory segments.

-q, --queues
    Write information about active message queues.

-s, --semaphores
    Write information about active semaphore sets.

-a, --all
    Write information about all three resources (default).
```

ipcrm 删除 System V IPC 的各种设施。

```shell
-M, --shmem-key shmkey
    Remove the shared memory segment created with shmkey after the last detach is performed.

-m, --shmem-id shmid
    Remove the shared memory segment identified by shmid after the last detach is performed.

-Q, --queue-key msgkey
    Remove the message queue created with msgkey.

-q, --queue-id msgid
    Remove the message queue identified by msgid.

-S, --semaphore-key semkey
    Remove the semaphore created with semkey.

-s, --semaphore-id semid
    Remove the semaphore identified by semid.
```

## **scp/rcp**

借助 `ssh` 实现的文件传输工具。

```shell
scp [可选项] src dst
```

## **zip/unzip**

语法： zip 压缩文件.zip 目录或文件。
功能： 将目录或文件压缩成zip格式。
常用选项：

`-r` 递归处理，将指定目录下的所有文件和子目录一并处理。

## **ldd**
 
`ldd` 显示可执行程序链接的动态库。

```shell
ldd [可执行程序]
```

## **ulimit**

`ulimit` 为 shell 内建指令，可用来控制 shell 执行程序的资源。

常用选项：

`-a`：显示所有资源限制。

`-c`：设置core文件的大小限制。

`-d`：设置数据段大小限制。

`-f`：设置文件大小限制。

`-m`：设置内存大小限制。

`-n`：设置打开文件数限制。

`-p`：设置缓冲区大小限制。

`-s`：设置栈大小限制。

`-t`：设置CPU时间限制。
