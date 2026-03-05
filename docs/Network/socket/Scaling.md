# Scaling in the Linux Networking Stack

[ref docs](https://docs.kernel.org/networking/scaling.html#rps-configuration){target="_blank"}

本文档描述了Linux网络协议栈中的一组互补技术，旨在提高并行性并提升多处理器系统的性能。

主要包含一下技术：

- RSS: Receive Side Scaling

- RPS: Receive Packet Steering

- RFS: Receive Flow Steering

- Accelerated Receive Flow Steering（TODO）

- XPS: Transmit Packet Steering（TODO）

## RSS: Receive Side Scaling

现代网卡（NIC）都支持多接收和发送队列。NIC 通过对每个数据包应用过滤器进行分发，该过滤器将数据包分配到少数几个逻辑流之一。每个流的数据包被引导至独立的接收队列，进而可由不同的CPU进行处理。，该机制通常被称为“接收端扩展”（RSS）。


RSS中使用的过滤器通常是对网络层和/或传输层报头进行哈希处理的函数。例如，对数据包的IP地址和TCP端口进行四元组哈希。

```
                    ┌─────────────────────────┐
                    │         NIC网卡          │
                    │(multiple Receive queues)│
                    └──────────┬──────────────┘
                               │
                        RSS hash(flow)
                               │
                ┌──────────────┴──────────────┐
                │                             │
            Queue 0                         Queue 1
                │                             │
              CPU 0                         CPU 1
    
```

支持多队列功能的网卡驱动程序通常提供一个内核模块参数，用于指定要配置的硬件队列数量。典型的RSS配置方案是：若设备支持足够的队列，则为每个CPU配置一个接收队列；否则至少为每个内存域配置一个队列，其中内存域指共享特定内存层级（如L1、L2、NUMA节点等）的CPU集合。

## RPS: Receive Packet Steering

Receive Packet Steering（RPS）在逻辑上是Receive Side Scaling（RSS）的软件实现。

由于涉及软件层，其必然在数据路径中被稍后调用。RSS负责选择队列（进而确定执行硬件中断处理程序的CPU），而 RPS 则选择执行中断处理程序之后进行协议处理的 CPU，这里可以理解为 RPS 选择负责收包的 CPU。这是通过将数据包放入目标CPU的后置队列并唤醒CPU进行处理来实现的。RPS相较于RSS具有以下优势：

- 它可与任何网卡配合使用（针对不支持 RSS 的网卡）

- 软件过滤器可轻松添加至新协议进行哈希处理

- 它不会增加硬件设备的中断频率（尽管它确实引入了处理器间中断（IPI））。

在接收中断处理程序的下半部分，当驱动程序通过 某些系统调用 将数据包向上传输至网络堆栈时，会调用 RPS，这些系统调用通过 get_rps_cpu() 选择应处理数据包的队列。每个接收硬件队列都关联着一组 CPU，RPS 可将数据包排入这些CPU队列进行处理。

```
                    ┌─────────────────────────┐
                    │         NIC网卡          │
                    │   (multiple RX queues)  │
                    └──────────┬──────────────┘
                               │
                        RSS hash(flow)
                               │
                ┌──────────────┴──────────────┐
                │                             │
         RX Queue 0                     RX Queue 1
                │                             │
                ▼                             ▼
           CPU 0 interrupt               CPU 1 interrupt
           (hard IRQ)                    (hard IRQ)
                │                             │
                ▼                             ▼
        schedule NET_RX_SOFTIRQ        schedule NET_RX_SOFTIRQ
                │                             │
                ▼                             ▼
           CPU0 softirq                  CPU1 softirq
         (net_rx_action)               (net_rx_action)
                │                             │
                │                             │
        ┌───────▼────────┐           ┌────────▼───────┐
        │     RPS?       │           │      RPS?      │
        │ Receive Packet │           │ Receive Packet │
        │   Steering     │           │   Steering     │
        └───────┬────────┘           └────────┬───────┘
                │                              │
      (optional redirect CPU)        (optional redirect CPU)
                │                              │
                ▼                              ▼
           CPU3 softirq                    CPU2 softirq
           (enqueue skb)                   (enqueue skb)
                │                              │
                ▼                              ▼
           TCP/IP stack                   TCP/IP stack
        (ip_rcv → tcp_v4_rcv)           (ip_rcv → tcp_v4_rcv)
                │                              │
                ▼                              ▼
          socket receive queue            socket receive queue
                │
                ▼
        wake up waiting thread
        (epoll / io_uring / poll)
                │
                ▼
        用户态网络线程 (User Thread)
           recv() / read()

```

可通过sysfs文件项为每个接收队列配置RPS可能转发流量的CPU列表：

```
/sys/class/net/<dev>/queues/rx-<n>/rps_cpus
```

该文件实现了一个CPU位图。当位图值为零（默认状态）时，RPS功能将被禁用，此时数据包将在中断发生的CPU上进行处理。

对于单队列设备，典型的RPS配置是将rps_cpus设置为与中断CPU处于同一内存域的CPU。对于多队列系统，若RSS配置为将硬件接收队列映射至每个CPU，则RPS可能冗余且无必要。若硬件队列数量少于CPU数量，且每个队列的rps_cpus与该队列中断CPU共享相同内存域时，启用RPS可能带来益处。

## RFS: Receive Flow Steering

虽然RPS仅基于哈希值引导数据包，因此通常能实现良好的负载分布，但它并未考虑应用程序的局部性。而接收流引导（RFS）则实现了这一功能。RFS的目标是通过将数据包的内核处理引导至正在运行该数据包的应用程序线程所在的CPU，从而提高数据缓存命中率。RFS依赖相同的RPS机制，既将数据包加入另一个CPU的后备队列，又唤醒该CPU（没懂？意思应该是将数据写入该 CPU 的接收队列，然后唤醒它处理数据包+执行应用代码，这样来保证处理数据包和执行应用层代码的是同一个CPU，我猜是这样）。


对于 RPS，只根据包的哈希值来确定处理包的CPU：

```
CPU = hash(flow) % num_cpu
```

但是应用线程和包处理线程可能在不同的CPU上运行，就会导致 cache miss。RFS的目标是 `packet CPU == application thread CPU`，它会维护一个 process flow → CPU 的映射表：

```
flow_table[index] = cpu_id
index   CPU
--------------
10      CPU3
11      CPU1
12      CPU1
13      CPU5
```

所以包分发流程就变成了：

```
packet arrives
     │
     ▼
flow hash
     │
     ▼
index = hash % table_size
     │
     ▼
cpu = flow_table[index]
```

flow_table 是动态更新的，最开始是空的。应用线程处理完成后，会记录该 flow 是由哪个 CPU 处理的，最开始是空表时，就直接使用 RPS 的哈希结果确定要执行处理的 CPU。整个 RFS 的流程如下：

```
NIC
 │
 ▼
RX queue
 │
 ▼
softirq
 │
 ▼
flow hash
 │
 ▼
lookup flow_table
 │
 ├─ 有CPU → send to that CPU
 │
 └─ 没CPU → fallback RPS
                │
                ▼
           hash → CPU
```

下面介绍 RFS 的实现方式：

`rps_sock_flow_table` 是一个 全局流表（global flow table），用于记录每个网络流应该被处理的 **目标 CPU** —— 即当前在 **用户态处理该流的 CPU**。

该表中的每个值都是一个 **CPU 索引**，并且会在系统调`recvmsg`，`sendmsg` 执行过程中更新，也就是说，当应用线程在某个 CPU 上调用 `recvmsg/sendmsg` 处理某个连接时，RFS 就会把 **这个 flow 映射到该 CPU**。


当调度器（scheduler）把一个线程 **从一个 CPU 迁移到另一个 CPU** 时，如果旧 CPU 上仍然有 **尚未处理的接收数据包**，就可能导致 **数据包乱序（out-of-order）**。为了避免这种情况，RFS 引入了 第二个流表 来跟踪每个流的 未完成数据包（outstanding packets）：

```
rps_dev_flow_table
```

这个表的特点：

**每个网络设备的每个 接收队列 都有一个独立的表**

每个表项包含：

  * 一个 **CPU index**
  * 一个 **counter**

其中：

* **CPU index**

  * 表示当前这个 flow 的数据包 **被投递到哪个 CPU 进行内核处理**

理想情况下：

```
用户态处理 CPU == 内核处理 CPU
```

也就是说：

```
rps_sock_flow_table CPU == rps_dev_flow_table CPU
```

但如果调度器刚刚把应用线程迁移到了新的 CPU，而内核中 **仍然有旧 CPU 上排队的数据包**，那么这两个 CPU 值就会暂时 **不一致**。

`rps_dev_flow_table` 中的 **counter** 用来记录：

> 当某个 flow 的数据包最后一次被加入某 CPU backlog 队列时，该队列的长度状态。

Linux 每个 CPU 都有一个 **backlog queue** 用来处理网络 softirq。

每个 backlog 队列维护两个计数器：

**head counter**

* 每当一个数据包 **被取出处理（dequeue）** 时递增

**tail counter**

```
tail = head + queue_length
```

因此：

```
rps_dev_flow[i].counter
```

记录的是：

> 当 flow i 的数据包被放入 CPU 队列时，该 flow 对应的数据包在该队列中的 **最后位置（tail）**。

换句话说：

`rps_dev_flow[i]` 记录的是：

> 当前 flow i 在指定 CPU backlog 队列中的 **最后一个排队数据包的位置**。

需要注意：

```
entry i 是通过 hash 选择的
```

因此**多个不同的 flow 可能映射到同一个 entry**


接下来就是 **RFS 防止数据包乱序的关键机制**。

当内核在 `get_rps_cpu()` 中为某个数据包选择 CPU 时，会同时查看两个表：

```
rps_sock_flow_table
rps_dev_flow_table
```

逻辑如下：

如果如果两个 CPU 相同，即：

```
rps_sock_flow_table CPU == rps_dev_flow_table CPU
```

说明：

* 用户态线程和内核处理在 **同一个 CPU**

此时：

```
数据包直接入队到该 CPU 的 backlog
```

如果两个 CPU 不同，说明：

* 应用线程 **可能已经迁移 CPU**
* 但内核 backlog 中 **仍然有旧 CPU 的数据包**

此时内核会检查是否可以 **安全迁移 flow 到新 CPU**。

只有满足以下条件之一时才允许迁移：

1. **以前的 flow 包已处理完**

```
current_cpu_queue.head >= rps_dev_flow[i].counter
```

意思是：

> 旧 CPU backlog 中 **该 flow 的数据包已经全部处理完**

因为：

```
head >= tail
```

说明队列里已经没有该 flow 的旧包。


2. **当前 CPU 未设置**

即：

```
current_cpu >= nr_cpu_ids
```

表示该 entry 还没有合法 CPU。


3. **当前 CPU 已经 offline**

例如 CPU 被 hotplug 下线。


如果满足以上任意条件：

```
rps_dev_flow_table CPU = rps_sock_flow_table CPU
```

即：**flow 被迁移到新的 CPU**






