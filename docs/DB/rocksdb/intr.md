

本章是我参考[官方文档](https://github.com/facebook/rocksdb/wiki){target=_blank}学习 RocksDB 的笔记。


## RocksDB

RocksDB 是一个具有键 KV 接口的存储引擎，其中键和值是任意字节流。它是一个 C++ 库。它是由 Facebook 基于 LevelDB 开发的，并为 LevelDB API提供向后兼容的支持。

RocksDB 支持各种存储硬件，最初的重点是 SSD。它使用 Log-Structured Merge（LSM）Tree 做为基本的数据存储结构，完全用C++编写，并有一个名为 RocksJava 的 Java 包装器。

## High Level Architecture

<figure markdown="span">
    ![Image title](./01.png){ width="850" }
</figure>