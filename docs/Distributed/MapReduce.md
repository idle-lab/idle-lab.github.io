
## **Lab1**

### Implement details

需要有两个部分：coordinator 和 worker

- coordinator 与 worker 间要使用 RPC 进行通信。

- 每个 worker 将在循环中向 coordinator 请求任务，从一个或多个文件读取任务的输入，执行任务，将任务的输出写入一个或更多文件，然后再次向 coordinator 询问新任务。

- 协调员应该注意到一名 worker 是否在合理的时间内没有完成任务（对于这个 lab1，用10秒），并将相同的任务交给另一名 worker。

- 当整个 MapReduce Job 完全完成时，所有的 worker 应退出。可以使用一个 "please exit" 类型的任务来实现。


