

## **技术面自我介绍**

面试官您好，我叫陈一波，是宁波大学计算机科学与技术专业26届学生。很荣幸能够参加贵公司今天的面试。接下来我想从我的相关项目经历和个人优势两方面来介绍一下我自己。

首先是项目经历：

第一个项目是现在正在做的 CMU 15445 的实验 bustub，一个面向磁盘的传统关系型数据库，主要实现了四个部分：

- 基于 LRU-K 的缓存池管理器，主要负责控制 Page 的进出以及脏页的持久化；

- 面向磁盘的可扩展哈希索引。

- 

后续还在实现中。实现到这里，我对数据库产生了浓厚的兴趣，之后非常想从事这方面的工作。


之后是个人优势：

1. 我熟悉 C++ 现代特性，如：智能指针、lambda 表达式，移动语义，模板编程等。熟悉数据结构与算法，常见的栈、队列、堆，进阶一点的并查集，线段树，树状数组，AVL 树。

2. 我的基础知识比较扎实，如：OS 的内存管理，进程管理，TCP/IP 协议栈的 http，tcp，udp 网络协议。

3. 我熟悉很多工具，如 git，Linux 命令行，gdb，markdown。我通过了 CET-4，可以阅读英文文档。我就是通过阅读 CMU 的英文文档，来完成 lab 的。

4. 我目标明确，自学能力强，提前学习了大三的编译原理、操作系统等课程。善于合作交流，乐于分享，我的个人网站就分享了我所学到的所有知识。

期待能有机会为贵公司贡献我的技能和经验，谢谢您的考虑。





## **可能的提问**

在第一轮技术面试中，结合您在简历中的经验和 Timeplus 的职位要求，我可能会问以下几个问题：

### C++和多线程编程


- 您提到熟悉C++和多线程编程，能否分享您在"单Reactor多线程高并发服务器"项目中遇到的主要挑战是什么？您是如何优化线程调度和资源管理的？

第一，由于第一次独自完成项目，在项目初期，由于没有预先规划完整的系统架构，导致命名混乱且不同的类有循环依赖的问题，我后续构造了一个基类，外提循坏依赖的部分来解决。第二，没有使用 Sanitizers 这样的错误检测工具，导致多线程项目难以调试，难以定位错误。

为了优化线程调度，我实现了一个基于C++17的线程池。线程池通过重用一组固定数量的线程，避免了频繁创建和销毁线程的开销，这显著提升了系统在高并发场景下的性能。同时，线程池还提供了任务队列，可以平衡任务负载，确保线程资源得到充分利用。
在资源管理方面，我采用了RAII（资源获取即初始化）的编程思想，比如封装Socket类，在Socket创建时，文件描述符由该类管理，并在对象析构时自动关闭文件描述符。这样不仅简化了资源管理，还有效避免了内存泄漏问题。


- 在项目中，如何处理多线程环境下的并发数据访问问题？您使用了哪些同步机制（如锁或无锁编程）？

当前我只是简单的用到互斥锁，来保证线程同步。

### 系统架构设计

- 您在开发基于单Reactor多线程的在线判题系统时，为什么选择这种架构？与其他架构（如多Reactor或事件驱动模型）相比有哪些优缺点？

单 Reactor 单执行流：

第一个缺点，因为只有一个进程，无法充分利用 多核 CPU 的性能；

第二个缺点，Handler 对象在业务处理时，整个进程是无法处理其他连接的事件的，如果业务处理耗时比较长，那么就造成响应的延迟；

单 Reactor 多线程：

优点：重复利用多核 CPU 性能；

缺点：因为一个 Reactor 对象承担所有事件的监听和响应，而且只在主线程中运行，在面对瞬间高并发的场景时，容易成为性能的瓶颈的地方

多 Reactor 多线程：

优点：没有了单 Reactor 多线程的瓶颈问题。

缺点：可能会有惊群现象。需要一些额外手段来解决，如：加锁，保证每次只有一个子进程进行 accept。

总的来说，单Reactor多线程架构适合中等并发的场景，并且在开发和维护上相对简单。然而，如果未来系统遇到更高的并发需求，我会考虑迁移到多Reactor多线程架构，以进一步提升系统的扩展性，同时使用负载均衡和锁机制解决惊群问题。


- 如何确保系统的高并发性和稳定性？能否详细说明您的日志记录系统是如何帮助您监控和诊断系统问题的？



### Linux和网络编程

- 您熟悉Linux的I/O复用技术，在实际应用中如何使用`epoll`提升系统性能？能否具体解释其工作原理及如何管理大量的文件描述符？

通过 epoll 进行多路复用，主线程不断检测是否有就绪的文件描述符，当有就绪的文件描述符就交给线程池处理。避免了 IO 的阻塞，能更快响应请求。

使用 epoll 时，主要是调用三个函数：`epoll_create`、`epoll_ctl`、`epoll_wait`。

创建一个 epoll 句柄，epoll_ctl 可以向 epoll 中添加关系的文件描述符以及对应事件。调用 epoll_wait 会返回就绪事件，将其交给线程池处理即可。

epoll 在内核中维护了一个红黑树和一个链表，红黑树中记录关心的文件描述符，链表记录就绪的文件描述符，当有就绪事件产生，就会将文件描述符加入到就绪链表中，当我们调用 epoll_wait 函数是，就会将就绪事件拷贝到用户态中。

此外，结合边缘触发（ET）模式可以减少不必要的重复通知，进一步提升性能。

- 在Socket编程中，如何处理长连接（HTTP持久连接）中的资源泄漏或连接超时问题？

我使用了 RAII 的思想，将 socket 进行封装，并且为每个连接维护一个时间戳，每次分发完任务后，检查是否有连接已经超时，如果超时就关闭 socket 文件描述符。

### 数据库和性能优化

- 您提到在BUSTUB数据库项目中实现了LRU-K替换策略，能否详细介绍一下LRU-K的工作机制以及您是如何优化Buffer Pool Manager的？

LRU-K 会为每个页面维护 k 个时间戳。在替换页面时，考虑的时第 k 次访问距现在的时间差，时间差越大，说明最近访问的越少，每次踢出时间差最大的。

当存在未访问够 k 的页面，这个距离就是正无穷。当有多个正无穷时，采用 FIFO 的策略。

LRU-K 时对 LRU 的优化，考虑了访问频率的因素。

- 您是如何通过细化锁的粒度来提升系统性能的？能否具体举例说明在什么场景下应用了这一技术？


这些问题不仅会考察您的技术能力，还能了解您如何思考复杂的系统设计、调优和实际项目中遇到的问题解决能力。同时，您丰富的竞赛经历（如ACM和ICPC）也说明了您的算法和编程能力，我也会适当询问一些算法相关的问题。


## **HR 面自我介绍**

面试官您好，我叫陈一波，是宁波大学计算机科学与技术专业26届学生。很荣幸能够参加贵公司今天的面试。接下来我从两个方面介绍一下我自己：


1. 第一点，我的技术栈和正在学习的方向：我比较擅长的领域是 C++ 开发，熟悉数据结构和算法，熟悉 Linux 操作系统，也掌握 C++ 的多线程编程技术，了解一些性能调优的知识。目前正在学习数据库开发相关的知识，也就是 CMU 的数据库导论。

2. 第二点，我的性格：我目标明确，能坚持下来做一件事。对代码精益求精，追求写出简洁优美的代码，也喜欢研究技术。我乐于分享，善于总结，我每学完一个新算法、新知识都会总结、并写一篇博客，所有现在我的个人博客网站上，基本涵盖了我所学的所有知识。

希望可以加入贵公司，感谢您的考虑。



- 为什么来上海？

上海离宁波比较近，我哥也在上海读书，可以有个照应。

- 为什么想实习？

学校里知识只停留在课本，课程实验也比较简单，想了解一些工程上是如何应用的。

- 住哪？

两个月青旅，之后租房。

- 为什么想来？

个人对数据库比较感兴趣，看了贵公司 gihub 上的项目，也比较感兴趣，想深入了解一下。

- timplus Proton

一个流式的 sql 引擎，可以通过传统的 SQL 语句做一些实时的数据分析，并转发，类似中间件。

## **反问**


- 新人培训是如何进行的？


- 公司留用实习生的标准是什么？