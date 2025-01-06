

本章节博客是 CMU 15-445 2023 Fall 的课程笔记，一些学习资源：

- [课程主页](https://15445.courses.cs.cmu.edu/fall2023/){targt=_blank}

- [【数据库系统导论 15-445 2023Fall】CMU—中英字幕](https://www.bilibili.com/video/BV1Ex4y1p7bi?p=1&vd_source=0de771c86d90f02a6cab8152f6aa173f){targt=_blank}

- [open-courses](https://zhenghe.gitbook.io/open-courses/cmu-15-445-645-database-systems/relational-data-model){targt=_blank} 

也有关于课程项目 BUSTUB 的一些编写思路。一些参考：  

- [BusTub 养成记：从课程项目到 SQL 数据库](https://zhuanlan.zhihu.com/p/570917775){targt=_blank}


- [CMU15-445 2023 Fall Project0~4 通关总结](https://zhuanlan.zhihu.com/p/679980578){targt=_blank}

- [xxl CMU15445(2023-fall)](https://4ever-xxxl.github.io/categories/cmu154452023-fall/){targt=_blank}

## **关于 BUSTUB**

这个课程实验用的是 C++17，需要一些 C++ 基础才能做。我写的是 2023 Fall 这一期的：

- **Project 0 -- C++Primer**：它是检验我们 C++17 语法水平的，要写一个 Copy-Write 的 KV 存储字典树。

- **Project 1 -- Buffer Pool**：一个 Buffer Pool Manager，管理整个 DB 的页面，对于并发控制，优化了锁的粒度，使用 2PL 协议，并且以相同顺序申请锁避免死锁。对于 I/O，实现了多线程写优化。最终 Leaderboard 排名 48。

- **Project 2 -- Extensible Hash Index**：实现一个可扩展哈希索引，使用 RAII 来管理哈希表的页面释放，这部分主要是优化了锁的粒度，其他只要实现大差不差，性能差别主要还是在 buffer pool，最终 Leaderboard 排名 34。

- **Project 3 -- Query Execution**：基于 Iterator Model 实现查询层的算子，包括SeqScan、IndexScan、Insert、Update、Delete、HashJoin，Aggregation和Windowfunction。以及一些 Heuristics 的优化，包括：SeqScan to IndexScan、NestedLoopJoin to HashJoin、Top-N Optimization。

- **Project#4 -- Concurrency Control**：实现 MV + OCC 的并发控制协议，实现了 Snapshot level 的隔离，支持 read-write、write-read、read-read 的并行执行。

下面是整个项目的整体结构：

<figure markdown="span">
  ![Image title](./bustub-structure.svg){ width="600" }
</figure>

## **论文**

- [Are You Sure You Want to Use MMAP in Your Database Management System?](https://db.cs.cmu.edu/papers/2022/cidr2022-p13-crotty.pdf){target=_blank}：mmap 映射文件引发的一些问题。

- [Exploiting Directly-Attached NVMe Arrays in DBMS](https://www.cidrdb.org/cidr2020/papers/p16-haas-cidr20.pdf){target=_blank}：现代 SSD 替代传统 HDD 的优势。

