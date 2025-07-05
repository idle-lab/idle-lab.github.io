

该库实现了 raft 算法核心的处理逻辑。

具体如何存储 raft log、raft state 都由用户实现，如何实现节点间通信也由用户实现。

在 etcd 中，为 raft log、raft state 的存储实现了 WAL。节点间的通信使用 http 协议，实现的逻辑在 etcdserver/api/rafthttp 目录下，节点通信还实现了 pipeline、stream 等优化。

