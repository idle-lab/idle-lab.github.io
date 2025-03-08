
## Implementation

首先将输入文件分成 M 份大小为 16MB~64MB 的可以并行处理的子文件，将 M 份的数据输入 MapReduce 程序，随后程序按照下图所示的执行一下几步：

![alt text](image-1.png)


1. 在用户程序中的 MapReduce 库会启动很多子程序，其中有一个特殊的进程 -- Master，其余的进程都是 Worker。Master 会为每个空闲的 Worker 分配 map 或 reduce 任务。

2. 被分配 map 任务的 Worker 会读取对应输入子文件的内容。从输入数据中解析出 key/value 对，并将其输入用户定义的 Map 函数中。用户 Map 函数产生的 key/value 对会被缓存到内存中。

3. 被缓存的键值对会被周期地写入本地磁盘中，输出的数据通过分区函数被分成 R 个部分。然后这 R 个部分的数据会被返回给 Master，后续会负责将这些位置交给执行 reduce 任务的 Worker。

4. 

## **Lab1**

需要有两个部分：coordinator 和 worker

- coordinator 与 worker 间要使用 RPC 进行通信。

- 每个 worker 将在循环中向 coordinator 请求任务，从一个或多个文件读取任务的输入，执行任务，将任务的输出写入一个或更多文件，然后再次向 coordinator 询问新任务。

- 协调员应该注意到一名 worker 是否在合理的时间内没有完成任务（对于这个 lab1，用10秒），并将相同的任务交给另一名 worker。

- 当整个 MapReduce Job 完全完成时，所有的 worker 应退出。可以使用一个 "please exit" 类型的任务来实现。

- 
