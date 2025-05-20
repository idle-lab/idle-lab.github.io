
[论文原文](https://www.usenix.org/legacy/event/atc10/tech/full_papers/Hunt.pdf){target=_blank}



本文的主要贡献如下:

- Coordination kernel：提出了一种具有宽松一致性保证的无等待协调服务，用于分布式系统中。特别地，描述了协调内核的设计和实现，已在许多关键应用程序中使用它来实现各种协调技术。

- Coordination recipes：展示了如何使用 ZooKeeper 构建更高级别的协调原语，甚至是阻塞和强一致性原语，这些原语通常用于分布式应用程序中。

- Experience with Coordination：分享了使用 ZooKeeper 的一些方法，并评估了它的性能。