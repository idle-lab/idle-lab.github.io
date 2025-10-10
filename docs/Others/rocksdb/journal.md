
## **Journal**

日志是用来描述数据系统历史状态的元数据。

日志也是 rocksdb 完整性和恢复的关键，rocksdb 中有两种类型的日志：

- WAL（Write Ahead Log）：WAL 用于保证内存的状态
  
-  MANIFEST：用于保证磁盘的状态。

rocksdb 的事务模型是依赖于日志的，尤其是 WAL，用于事务 commit、recovery 和 abort。

## **WAL**

对于 rocksdb 中的每次更新，都要写入两个地方：

- 一个内存数据结构，就是我们之前提到的 memtable。

- 写日志（WAL）到磁盘。

当机器崩溃时，WAL 可以用来完成恢复 memtable 中的数据。在默认的配置下，rocksdb 通过每次用户更新都刷新 WAL 来保证 process crash consistency。


### **WAL 的生命周期**



