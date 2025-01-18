
## **Distributed DBMSs**

分布式 DBMS 将单个逻辑数据库划分为多个物理资源。应用程序（通常）不知道数据分布在不同的硬件上。该系统依赖于单节点DBMS的技术和算法来支持分布式环境中的事务处理和查询执行。设计分布式 DBMS 的一个重要目标是容错（即避免单个节点故障导致整个系统瘫痪）。

当我们在谈论分布式数据库时，首先要明确什么是分布式系统：如果通信成本和通信的可靠性问题不可忽略，就是分布式系统。这也是区分 Parallel DBMS 和 Distributed DBMS 的依据所在：

|Parallel DBMSs|Distributed DBMSs|
|:-:|:-:|
|不同节点在物理上隔得很近|不同节点在物理上可能隔得很远
|不同节点通过高速局域网连接|不同节点通过普通公共网络相连接
|通信成本很小，基本不会产生问题|通信成本和通信问题不可忽略

<hr>

## **System Architectures**

DBMS 的系统架构指定了 CPU 可以直接访问哪些共享资源。它影响 CPU 之间的协调方式以及它们在数据库中检索和存储对象的位置。

单节点 DBMS 使用所谓的 shared everything 架构。该单个节点使用其自己的本地内存地址空间和磁盘在本地 CPU 上执行工作程序。还有其他三种分布式架构：Shared Memory、Shared Disk、Shared Nothing：

<figure markdown="span">
    ![Image title](./01.png){ width="750" }
</figure>


