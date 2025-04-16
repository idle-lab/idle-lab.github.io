[论文原文](https://pdos.csail.mit.edu/6.824/papers/raft-extended.pdf){target=\_blank}

## 1 Introduction

共识算法允许机器集合作为一个连贯的群体工作，即使其中一些成员出现故障也能继续工作。因为这一点，它们在构建可靠的大规模软件系统中扮演者关键角色。Paxos 在过去的十年中主导了共识算法的讨论，不幸的是，Paxos 相当难以理解，尽管有很多使其更易接受的尝试。另外，其架构需要复杂的修改以支持实用的系统。其结果是，系统构建者和学生都很受 Paxos 困扰。

因此，我们以可理解性为主要目标，设计了一套新的共识算法，即：**Raft**。Raft 与现有的共识算法在很多方面都很相似，但 Raft 有很多新特性：

-   强 leader： 与其它共识算法相比，Raft 使用了更强的领导权形式。例如，日志条目(log entry)仅从 leader 流向其它服务器。这简化了对多副本日志的管理，并使 Raft 更容易理解。

-   领导选举： Raft 使用随机计时器来选举 leader。这仅在任何共识算法都需要的心跳机制上增加了很小的机制，但能够简单又快速地解决冲突。

-   成员变更： Raft 用来变更集群中服务器集合的机制使用了一个新的*联合共识（joint consensus）*方法，其两个不同配置中的大多数服务器会在切换间有重叠。这让集群能够在配置变更时正常地继续操作。

我们认为，无论为了教育目的还是作为实现的基础，Raft 都比 Paxos 和其它共识算法更优秀；Raft 的描述足够完整，能够满足使用系统的需求；Raft 有很多开源实现并已经被一些公司使用；Raft 的安全性性质已经被形式化定义并证明；Raft 的效率与其它算法相似。

## 2 Replicated state machines

## 5 The Raft consensus algorithm

### 5.1 Raft basics

一个 Raft 集群由多个 Server 组成，通常是 5 个，这样的系统可以容忍两个 Server 崩溃。在任何时间，每个 Server 只能是以下三种状态之一：leader, follower, or candidate。正常运行时，一个集群只有一个 leader，其他 Server 都是 follower。 follower 不会主动提出任何请求，只是被动响应 leader。leader 会处理所有客户端请求（如果 follower 收到客户端请求会将其重定向到 leader）。candidate 状态是为了选举一个新的 leader，会在 5.2 节介绍。状态间的转换如下图：

<figure markdown="span">
    ![Image title](./03.png){ width="450" }
</figure>

Raft 将时间划分长度不定的任期，如下图：任期使用连续的整数来编号。

<figure markdown="span">
    ![Image title](./02.png){ width="450" }
</figure>

每个任期由选举开始，其中一个或更多的 candidate 试图成为 leader，5.2 节介绍。在某些情况下，选举会出现票数相等的情况，这种情况任期会直接结束，新的任期很快会重新开始（通过一次新的选举）。Raft 保证一个任期内最多有一个 leader。

不同的服务器可能会在不同的时间观察到任期的转换，在一些情况下一个节点甚至可能错过整个任期，任期作为 Raft 的逻辑时钟，可以帮助 Server 检测旧信息以及过时的 leader。**每个 Server 内都会维护一个单调递增的 current term 字段，在 Server 间的所有通信都会带有该字段。**

Raft 节点间使用 RPC 进行通信，最基本的 Raft 共识算法只需要两种 RPC：

-   RequestVote：该调用会在选举期间被 candidates 调用，用途是拉票；

-   AppendEntries：该调用会在正常运行期间被 leader 调用，用途是复制 log 以及实现心跳机制。

在第 7 节中引入了第三种 RPC 调用，用于在 Server 间传递快照。当请求失败时 Server 会及时重试，并且为了性能，请求都是并行进行的。

### 5.2 Leader election

Raft 通过心跳机制来触发选举。当集群启动时，所有 Server 都是 follower，Server 会维持 follower 状态，如果一段时间（election timeout）内没收到来自 leader 的心跳，它就假设 leader 已经不可达来，然后开启一个新的选举去选择一个新的 leader。leader 会周期的向每个 follower 发送心跳（空 log entries 的 AppendEntries 调用），来维护自身的权威。

开始选举时，follower 会自增其 current term 字段，然后将其状态改变为 candidate。然后并行的向集群中的其他节点发起 RequestVote 调用，candidate 会一直持续到出现下面三种情况之一：

1. 它赢得了选举：如果候选者在同一任期内获得整个集群中大多数服务器的投票，则该候选者将赢得选举。每个服务器在给定任期内最多为一名候选人投票，先到先得（第 5.4 节增加了对投票的额外限制）。多数票规则确保在某一任期内最多只有一名候选人能够赢得选举。一旦某个候选者赢得选举，它就会成为领导者。然后，它就会向所有其他服务器发送心跳信息，以确立自己的权威，防止出现新的选举。

2. 其他节点成为了 leader：在等待选票时，candidate 可能会收到来自 leader 的 AppendEntries 调用，如果该 leader 的任期号大于等于的 candidate 的任期，candidate 就会意识到已经有人是 leader 了，他会变回 follower 状态；否则它会拒绝该请求，然后继续保持 candidate 状态。

3. 一段时间过去但没有 leader 出现：当同时有多个 follower 变为 candidate，就会导致选票分散，最终谁都不能得到大多数都选票，当这种情况发生时，每个候选人都会超时，并通过增加任期和启动新一轮请求投票 RPC 开始新的选举。但仅仅重试，可能会一直重复上述情况。**Raft 采用随机选举超时机制来解决。**
