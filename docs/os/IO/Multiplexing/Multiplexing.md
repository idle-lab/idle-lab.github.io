

所谓多路复用就是允许一个执行流同时监控多个文件描述符（包括套接字、文件、管道等），从而在任何一个或多个文件描述符准备好进行 I/O 操作时得到通知。Linux 为实现多路复用提供了三个系统调用 select、poll 和 epoll，下面一一介绍。

??? question "如何服务更多的用户？"
    我们如何能在有限的资源下服务更多的客户端？这就要提到 C10K 问题，即如何让一个 2GB 内存的服务器响应 10k 个客户端（**C**lient）的并发请求。要充分利用服务器性能，选用一个高效的 IO 模型是必须的。

    在网络套接字部分，对于服务多用户的问题，我们的解决办法是多线程，即多执行流（线程/进程）模型，每个线程/进程单独服务一个 TCP 连接，但当一个用户没有输入时，这个线程就只能阻塞，这无疑是对资源的浪费。

    而本文介绍的多路复用，是一种资源消耗更少更高效的 IO 模型。


## **select**

`select` 实现多路复用的方式是，将已连接的 Socket 都放到一个文件描述符集合，然后调用 select 函数将文件描述符集合拷贝到内核里，让内核来检查是否有网络事件产生，检查的方式很粗暴，就是通过遍历文件描述符集合的方式，当检查到有事件产生后，将此 Socket 标记为可读或可写， 接着再把整个文件描述符集合拷贝回用户态里，然后用户态还需要再通过遍历的方法找到可读或可写的 Socket，然后再对其处理。

### **系统调用**

```c
#include <sys/select.h>

int select(int nfds, fd_set *readfds, fd_set *writefds, 
                    fd_set *exceptfds, struct timeval *timeout);
```

**参数说明**：

- **nfds**:整数值，是所有文件描述符的范围，即最大文件描述符加 1。通常可以通过 `FD_SETSIZE` 获取。
   
- **readfds**:指向 `fd_set` 结构的指针，表示需要监视是否有数据可读的文件描述符集合。如果不需要监视任何读事件，可以传递 NULL。
   
- **writefds**:指向 `fd_set` 结构的指针，表示需要监视是否可以写入数据的文件描述符集合。如果不需要监视任何写事件，可以传递 NULL。
   
- **exceptfds**:指向 `fd_set` 结构的指针，表示需要监视异常条件（如带外数据）的文件描述符集合。如果不需要监视任何异常事件，可以传递 NULL。
   
- **timeout**:指向 `timeval` 结构的指针，用于指定 `select` 的超时时间。可以为 NULL 表示无限期等待。`timeval` 结构定义了秒和微秒级别的超时值。

**返回值**：

- 成功时返回就绪的文件描述符的数量。

- 超时返回 0。

- 失败时返回 -1，并设置 `errno`。

**`fd_set` 操作**

`fd_set` 是一个集合，用于存储多个文件描述符。以下是操作 `fd_set` 的一些宏：

- **FD_ZERO(fd_set *set)**: 清空 `fd_set` 集合。

- **FD_SET(int fd, fd_set *set)**: 将文件描述符 `fd` 加入 `fd_set` 集合。

- **FD_CLR(int fd, fd_set *set)**: 从 `fd_set` 集合中移除文件描述符 `fd`。

- **FD_ISSET(int fd, fd_set *set)**: 检查文件描述符 `fd` 是否在 `fd_set` 集合中。


??? code "select server"
    === "select_server.hpp"

        ```cpp
        #pragma once
        #include "socket.hpp"
        #include "log.hpp"
        #include <vector>

        // Linux
        #include <sys/select.h>

        #define DEFAULT_PORT 8888                     // 默认端口
        #define SELECT_SET_MAXSIZE sizeof(fd_set) * 8 // select 默认可维护的最多 socket 数量

        class select_server
        {
            void handle_events(fd_set &rfds) noexcept(false) // 处理读事件
            {
                // 处理listen socket
                if (FD_ISSET(_M_rfds[0]->get_fd(), &rfds))
                {
                    for (int i = 1; i < SELECT_SET_MAXSIZE; ++i)
                    {
                        if (_M_rfds[i] != nullptr) continue;
                        _M_rfds[i] = _M_rfds[0]->accept();
                        LOG(INFO,"Connect with cilent[%s,%d]\n",_M_rfds[i]->get_ip().c_str(),_M_rfds[i]->get_port());
                        break;
                    }
                }
                for (int i = 1; i < SELECT_SET_MAXSIZE; ++i)
                {
                    if (_M_rfds[i] == nullptr or !FD_ISSET(_M_rfds[i]->get_fd(), &rfds))
                        continue;

                    std::string _msg;
                    ssize_t n = _M_rfds[i]->recv(_msg);
                    if (n > 0)
                    {
                        LOG(INFO, "Client say#%s\n", _msg.c_str());
                        _M_rfds[i]->send("Hello");
                    }
                    else if (n == 0) // 客户端关闭 socket，删除对应 socket
                    {
                        LOG(INFO, "Client close socket\n");
                        _M_rfds[i]->close();
                        _M_rfds[i].reset();  // 释放资源
                        continue;
                    }
                    else // recv 错误
                    {
                        LOG(WARNING, "recv error with errno:%d\n", errno);
                        continue;
                    }
                }
            }

            std::vector<std::unique_ptr<tcp_sock>> _M_rfds; // read socket 数组
            bool is_running;
            uint16_t _M_port;
            std::string _M_ip;
        public:
            select_server()
                : is_running(false)
                , _M_rfds(SELECT_SET_MAXSIZE)
            { }

            void init(const std::string& ip = "0.0.0.0",uint16_t port = DEFAULT_PORT,int que_len = 5)
            {
                _M_port = port;
                _M_ip = ip;
                // 创建 listen 套接字
                _M_rfds[0].reset(new tcp_sock);
                try
                {
                    _M_rfds[0]->bind(ip, port);
                    int val = 1;
                    _M_rfds[0]->set_opt(SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)); // 地址复用
                    _M_rfds[0]->listen(que_len);
                }
                catch (const char *exp)
                {
                    std::cerr << exp << std::endl;
                }
            }


            void start() noexcept(false)
            {
                is_running = true;
                LOG(INFO,"Serve open in addr[%s,%d]\n",_M_ip.c_str(),_M_port);
                while (is_running)
                {
                    fd_set fds;
                    FD_ZERO(&fds);
                    int max_fd = -1;
                    for (auto &sock_ptr : _M_rfds)
                    {
                        if (sock_ptr == nullptr)
                            continue;
                        FD_SET(sock_ptr->get_fd(), &fds);
                        max_fd = std::max(max_fd, sock_ptr->get_fd());
                    }
                    // {s,ms} 设置超时事件，阻塞 timeout 的时间后，结束函数，如果有就绪的事件就提前结束，并返回剩余时间
                    timeval timeout = {5, 0};
                    int n = ::select(max_fd + 1, &fds, nullptr, nullptr, &timeout);

                    try
                    {
                        if (n < 0)  // 多路复用出错
                        {
                            LOG(ERROR, "Multiplexing error with errno:%d\n", errno);
                            throw "Multiplexing error";
                        }
                        if (n == 0) // 超时
                        {   
                            LOG(INFO, "Multiplexing Timeout\n");
                            continue;
                        }
                        LOG(INFO,"Have event,timeout left: %d.%d s\n",timeout.tv_sec,timeout.tv_usec);
                        handle_events(fds);
                    }
                    catch (const char *exp)
                    {
                        std::cerr << exp << std::endl;
                        exit(-1);
                    }
                }
            }
            void stop() noexcept { is_running = false; }
        };
        ```

    === "socket.hpp"

        ```cpp
        #pragma once
        #include <string>
        #include <cstring>
        #include <memory>
        #include "log.hpp"
        #include <sys/types.h>     
        #include <sys/socket.h>
        #include <netinet/in.h>
        #include <arpa/inet.h>

        // Linux
        #include <unistd.h>


        // socket 父类
        class sock
        {
        protected:
            int _M_fd = -1;
            uint16_t _M_port;
            std::string _M_ip;
            virtual void create_sockaddr_in(const std::string& ip,uint16_t port,sockaddr_in& addr/*输出型参数*/) = 0;
        public:
            sock(int domain,int type,int protocol) noexcept(false)
                :_M_fd(socket(domain,type,protocol)) 
            { 
                if(_M_fd < 0)
                {
                    LOG(ERROR,"socket error with errno:%d",errno);
                    throw "socket error";
                }
            }
            explicit sock(int sockfd)
                :_M_fd(sockfd)
            { }

            virtual void bind(const std::string& ip,uint32_t port) = 0;
            virtual ssize_t send(const std::string&,int) = 0;
            virtual ssize_t recv(std::string&,int) = 0;
            virtual void set_opt(int,int,const void*,socklen_t) = 0;
            virtual void get_opt(int, int,void *, socklen_t*) = 0;
            int get_fd() const noexcept                 {return _M_fd;}
            uint16_t get_port() const noexcept          { return _M_port; }
            const std::string& get_ip() const noexcept  { return _M_ip; }
            void close() { ::close(_M_fd); }
        };

        // 封装 tcp socket 系统调用
        // 维护 socketfd ，ip，port 
        class tcp_sock: public sock
        {
            // ip + port 创建 sockaddr_in
            void create_sockaddr_in(const std::string& ip,uint16_t port,sockaddr_in& addr/* 输出型参数 */)
            {
                memset(&addr,0,sizeof(addr));
                addr.sin_family = AF_INET;
                addr.sin_port = htons(port);
                inet_pton(AF_INET,ip.c_str(),static_cast<void*>(&addr.sin_addr.s_addr));
            }
        public:
            tcp_sock()
                :sock(AF_INET,SOCK_STREAM,IPPROTO_TCP)
            { }

            explicit tcp_sock(int sockfd)   // 通过文件描述符构造 socket
                :sock(sockfd)
            { }

            // 主机字节序的 port
            virtual void bind(const std::string& ip,uint32_t port) noexcept(false)
            {
                // 创建 ip + port 地址
                sockaddr_in _locl;
                create_sockaddr_in(ip,port,_locl);
                if(::bind(_M_fd,(sockaddr*)&_locl,sizeof(sockaddr_in)) < 0) // 绑定
                {
                    LOG(WARNING,"bind error with errno:%d",errno);
                    throw "bind error";
                }
                _M_ip = ip,_M_port = port;
            }

            
            virtual ssize_t send(const std::string& send_str,int flags = 0)
            {
                return ::send(_M_fd,send_str.c_str(),send_str.size(),flags);
            }


            virtual ssize_t recv(std::string& ret_str,int flags = 0)
            {
                char buff[2048];
                ssize_t n = ::recv(_M_fd,buff,sizeof(buff) - 1,flags);
                if(n <= 0)return n;

                buff[n] = 0;
                ret_str = buff;
                return n;
            }

            virtual void set_opt(int level, int optname, const void *optval, socklen_t optlen) noexcept(false)
            {
                if(::setsockopt(_M_fd,level,optname,optval,optlen) < 0)
                {
                    LOG(WARNING,"setsockopt error with errno:%d",errno);
                    throw "setsockopt error";
                }
            }

            virtual void get_opt(int level, int optname,void *optval, socklen_t *optlen) noexcept(false)
            {
                if(::getsockopt(_M_fd,level,optname,optval,optlen))
                {
                    LOG(WARNING,"getsockopt error with errno:%d",errno);
                    throw "getsockopt error";
                }
            }

            void listen(int que_len = 5) noexcept(false)
            {
                if(::listen(_M_fd,que_len) < 0)
                {
                    LOG(ERROR,"Server listen error with errno:%d\n",errno);
                    throw "Server error";
                }
            }

            std::unique_ptr<tcp_sock> accept() noexcept(false)
            {
                sockaddr_in _client;
                socklen_t _len = sizeof(sockaddr_in);
                memset(&_client,0,sizeof(_client));

                int nfd = ::accept(_M_fd,(sockaddr*)&_client,&_len);
                if(nfd < 0)
                {
                    LOG(WARNING,"accept error with errno:%d\n",errno);
                    throw "accept error";
                }
                // 创建 tcp socket
                std::unique_ptr<tcp_sock>sptr(new tcp_sock(nfd));
                sptr->_M_port = ntohs(_client.sin_port);
                char buff[128] = { 0 };
                inet_ntop(AF_INET,&_client.sin_addr.s_addr,buff,sizeof(buff));
                sptr->_M_ip = buff;
                return sptr;
            }

            void connect(const std::string& ip,uint32_t port) noexcept(false)
            {
                sockaddr_in _server;
                create_sockaddr_in(ip,port,_server);

                if(::connect(_M_fd,(sockaddr*)&_server,sizeof(sockaddr_in)) < 0)
                {
                    LOG(WARNING,"connect error with errno:%d",errno);
                    throw "connect error";
                }
            }
        };
        ```
    
    === "log.hpp"

        ```cpp
        #pragma once
        #include <string>
        #include <iostream>
        #include <ctime>
        #include <mutex>
        #include <cstdarg>

        enum
        {
            INFO,
            DEGUB,
            WARNING,
            ERROR,
            FATAL,
        };

        /**
        * @brief 一个简单的日志系统。线程安全。向 stderr 输出日志。
        */
        class logger
        {
            std::mutex _log_mt;
            logger() = default;
        public:
            // 饿汉单例
            static logger &get()
            {
                static logger *lg = new logger();
                return *lg;
            }

            logger(const logger &) = delete;
            logger &operator=(const logger &) = delete;

            void write_log(const std::string &_level, const std::string &_file, int _line, const char *format, ...)
            {
                std::string _msg;

                // 获取时间戳，并格式化
                std::time_t __t = time(nullptr);
                tm *_tm = gmtime(&__t);
                char _tm_buff[128];
                // struct tm
                // {
                //     int tm_sec;  /*秒，正常范围0-59， 但允许至61*/
                //     int tm_min;  /*分钟，0-59*/
                //     int tm_hour; /*小时， 0-23*/
                //     int tm_mday; /*日，即一个月中的第几天，1-31*/
                //     int tm_mon;  /*月， 从一月算起，0-11*/  1+p->tm_mon;
                //     int tm_year;  /*年， 从1900至今已经多少年*/  1900＋ p->tm_year;
                //     int tm_wday; /*星期，一周中的第几天， 从星期日算起，0-6*/
                //     int tm_yday; /*从今年1月1日到目前的天数，范围0-365*/
                //     int tm_isdst; /*日光节约时间的旗标*/
                // };
                snprintf(_tm_buff, sizeof(_tm_buff), "%d-%d-%d-%d:%d:%d",
                        _tm->tm_year + 1900 /*年*/, _tm->tm_mon + 1 /*月*/,
                        _tm->tm_mday /*日*/, _tm->tm_hour /*小时*/,
                        _tm->tm_min /*分钟*/, _tm->tm_sec /*秒*/);
                _msg = "[" + _level + "]" + "[ " + _file + "\t]" +
                    "[" + std::to_string(_line) + "]" + "[" + _tm_buff + "]: ";

                char _log_buff[1024];

                // 获取可变参数
                va_list args;
                va_start(args, format);
                vsnprintf(_log_buff, sizeof(_log_buff), format, args); // 用 va_list 做参数，输出到 dest

                _msg += _log_buff;
                {
                    std::lock_guard<std::mutex> lg(_log_mt);
                    std::cout << _msg;
                }
            }
        };

        logger &__log = logger::get();

        /**
        * 输出日志消息
        *  INFO
            DEGUB
            WARNING
            ERROR
            FATAL
        */
        #define LOG(level, format, ...) __log.write_log(#level, __FILE__, __LINE__, format, ##__VA_ARGS__);
        ```

    === "main.cpp"

        ```cpp
        #include "select_server.hpp"
        int main()
        {
            select_server ss;
            ss.init();
            ss.start();
            return 0;
        }
        ```

### **优缺点**

select 唯一的优点就是相比其他 IO 方式，select 可以同时等待多个文件描述符。

select 是最早提出的多路复用方案，它有很多缺点：

- 每个 `fd_set` 参数即使输入参数，又是返回值，所以每次传参都要重新初始化 `fd_set`。

- 维护的文件描述符数量有上限，不能超过 `sizeof(fd_set) * 8`。

- 在内核态和用户态都要遍历一次 `fd_set`。用户态到内核态（传参）、内核态到用户态（返回）都要拷贝一次 `fd_set`。常数大，效率低下。


## **poll**

`poll` 使用动态数组的方式组织文件描述符，没有了文件描述符数量的上限，并且将输入输出做了分离，不需要每次传参都重新初始化文件描述符列表。

但 `poll` 和 `select` 本质上区别不大，都是线性结构，检测文件描述符是否就绪都要遍历。

### **系统调用**

```cpp
#include <poll.h>

int poll(struct pollfd *fds, nfds_t nfds, int timeout);


```

**参数说明**

- `fds`: 一个指向 `pollfd` 结构体数组的指针。每个 `pollfd` 结构体表示一个文件描述符及其请求的事件。

- `nfds`: `pollfd` 结构体数组的大小，即文件描述符的数量。

- `timeout`: 等待的毫秒数。如果 `timeout` 为负值，`poll` 将无限期地等待；如果为 0，`poll` 将立即返回。

**返回值**

- 正数：已准备好的文件描述符数量。
- 0：超时，没有文件描述符准备好。
- -1：发生错误，可以通过 `errno` 获取错误代码。

**`pollfd` 结构体**

```cpp
struct pollfd {
    int   fd;         /* file descriptor */
    short events;     /* requested events */
    short revents;    /* returned events */
};
```

`fd`: 要监视的文件描述符。

`events`: 指定要监视的事件类型，常用事件包括：

  - `POLLIN`: 请求读事件。
  - `POLLOUT`: 请求写事件。

`revents`: `poll` 返回时，表示实际发生的事件类型。

  - `POLLIN`: 读事件就绪。
  - `POLLOUT`: 写事件就绪。
  - `POLLERR`: 错误条件，只能做 `revents`。
  - `POLLNVAL`: 文件描述符无效，只能做 `revents`。
  - `POLLHUP`: 表示文件挂起，如管道写端关闭，只能做 `revents`。


??? code "poll"
    其他文件与 select 相同，只有服务器实现不同。

    === "poll_server.hpp"
        ```cpp
        #pragma once
        #include "socket.hpp"
        #include "log.hpp"
        #include <vector>

        // Linux
        #include <poll.h>

        #define DEFAULT_PORT 8888    // 默认端口

        class poll_server
        {
            void handle_events() noexcept(false) // 处理读事件
            {
                // 处理listen socket
                if(_M_rfds[0].revents & POLLIN) // 读事件就绪
                {
                    auto sptr = _M_listener->accept();
                    
                    // 可以设置一个服务器处理事件上限，当超过上限就丢弃连接
                    _M_rfds.push_back({sptr->get_fd(),POLLIN,0});
                    LOG(INFO,"Connect with cilent[%s,%d]\n",sptr->get_ip().c_str(),sptr->get_ip());
                }
                for(int i = 1;i < _M_rfds.size();++i)
                {
                    if(!(_M_rfds[i].revents & POLLIN))continue; // 读事件就绪

                    char buff[1024];
                    ssize_t n = ::recv(_M_rfds[i].fd,buff,sizeof(buff) - 1,0);
                    if(n < 0)
                    {
                        LOG(WARNING,"Client recv error with errno:%d\n",errno);
                        continue;
                    }
                    if(n == 0)
                    {
                        LOG(INFO,"Client close socket\n");
                        std::swap(_M_rfds[i],_M_rfds.back());
                        ::close(_M_rfds.back().fd);
                        _M_rfds.pop_back();
                        continue;
                    }
                    // 返回一个数据
                    buff[n] = 0;
                    std::string _msg = "你好";
                    ::send(_M_rfds[i].fd,_msg.c_str(),_msg.size(),0);

                    LOG(INFO,"Client say#%s\n",buff);
                }

            }

            std::unique_ptr<tcp_sock> _M_listener;  // listen socket
            std::vector<pollfd> _M_rfds;    // read socket
            bool is_running;
            uint16_t _M_port;
            std::string _M_ip;
        public:
            poll_server()
                : is_running(false)
            { }

            void init(const std::string& ip = "0.0.0.0",uint16_t port = DEFAULT_PORT,int que_len = 5)
            {
                _M_port = port;
                _M_ip = ip;
                // 创建 listen 套接字
                _M_listener.reset(new tcp_sock);
                try
                {
                    _M_listener->bind(ip, port);
                    int val = 1;
                    _M_listener->set_opt(SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)); // 地址复用
                    _M_listener->listen(que_len);
                }
                catch (const char *exp)
                {
                    std::cerr << exp << std::endl;
                }
                _M_rfds.push_back({_M_listener->get_fd(),POLLIN,0});   // 关注 listen socket 的读事件。
            }


            void start() noexcept(false)
            {
                is_running = true;
                LOG(INFO,"Serve open in addr[%s,%d]\n",_M_ip.c_str(),_M_port);
                while (is_running)
                {
                    int timeout = 1000;    // 超时时间
                    int n = ::poll(_M_rfds.data(),_M_rfds.size(),timeout);

                    try
                    {
                        if (n < 0)  // 多路复用出错
                        {
                            LOG(ERROR, "Multiplexing error with errno:%d\n", errno);
                            throw "Multiplexing error";
                        }
                        if (n == 0) // 超时
                        {   
                            LOG(INFO, "Multiplexing Timeout\n");
                            continue;
                        }
                        LOG(INFO,"Have event\n");
                        handle_events();
                    }
                    catch (const char *exp)
                    {
                        std::cerr << exp << std::endl;
                        exit(-1);
                    }
                }
            }
            void stop() noexcept { is_running = false; }
        };
        ```

### **优缺点**

`poll` 相比 `select` 有以下优点：
`
- 没有文件描述符上限。

- 输出输出分离为 `events` 和 `revents` ，不需要重复初始化传入参数。

但 `select` 的有些缺点依然存在：

- `poll` 在检测事件是否就绪时依旧需要对 `pollfd` 数组进行遍历，用户态一次，内核态一次。

- 用户到内核（传参）、内核到用户（返回）依然要对参数 `pollfd` 进行两次拷贝。

不论是 `select` 还是 `poll`，随着文件句柄的增多，拷贝和遍历的消耗会不断增大，进而导致性能会显著下降。

## **epoll**

按照 man 手册的说法，epoll 是为处理大批量句柄而作了改进的 poll，它是在 2.5.44 内核中被引进的。它特别适用于处理大量并发连接，因其在管理大量文件描述符时表现出色。`epoll` 的设计旨在提高可扩展性和性能。

与 `select` 和 `poll` 不同，`epoll` 底层使用红黑树和一个双向队列实现。用户的文件描述符及对文件描述符的关注的事件由红黑树维护。就绪事件由双向队列维护。



### **系统调用**


**`epoll_create`**

```c
#include <sys/epoll.h>
int epoll_create(int size);
```

- 用于创建一个 `epoll` 实例。

- 返回一个 `epoll` 文件描述符。

自从linux2.6.8之后，size 参数是被忽略的，因为 size 是指监听的描述符个数， 现在内核支持动态扩展，该值的意义仅仅是初次分配的fd个数，后面空间不够时会动态扩容。

**`epoll_ctl`**

用于控制 `epoll` 实例，向其中添加、修改或删除文件描述符。
   
```c
#include <sys/epoll.h>
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
```

- **`epfd`**: `epoll` 文件描述符。

- **`op`**: 操作类型（`EPOLL_CTL_ADD`、`EPOLL_CTL_MOD`、`EPOLL_CTL_DEL`）。

- **`fd`**: 需要监视的文件描述符。

- **`event`**: 指向 `epoll_event` 结构的指针，描述感兴趣的事件和相关的数据。

**`epoll_wait`**

- 等待事件发生，并获取已就绪的文件描述符事件。

```c
#include <sys/epoll.h>
int epoll_wait(int epfd, struct epoll_event *events, 
                                int maxevents, int timeout);
```

- **`epfd`**: `epoll` 文件描述符。

- **`events`**: 用于存储已就绪事件的 `epoll_event` 数组。

- **`maxevents`**: 数组 `events` 的大小，即一次最多返回的事件数。

- **`timeout`**: 超时时间（毫秒），-1 表示无限等待，0 表示立即返回。

**`epoll_event` 和 `epoll_data`**

```c
struct epoll_event {
    uint32_t events;   // 事件类型
    epoll_data_t data; // 用户数据
};
typedef union epoll_data {
    void        *ptr;
    int          fd;
    uint32_t     u32;
    uint64_t     u64;
} epoll_data_t;
```

**`events`**: 事件类型，可以是以下值的组合：

- `EPOLLIN`: 表示对应的文件描述符可以读（包括对端 `SOCKET` 正常关闭）。

- `EPOLLOUT`: 表示对应的文件描述符可以写。

- `EPOLLPRI`: 表示对应的文件描述符有紧急数据可读（带外数据）。

- `EPOLLERR`: 表示对应的文件描述符发生错误。

- `EPOLLHUP`: 表示对应的文件描述符被挂断。

- `EPOLLET`: 将 `fd` 设置为边缘触发（Edge Triggered）模式。

- `EPOLLONESHOT`: 只监听一次事件，当监听完这次事件之后，如果还需要继续监听这个 `fd`，需要再次把这个 `fd` 加入到 `epoll` 队列里。

**`data`**: 用户数据，可以用来存储文件描述符或者其他用户定义的数据。


### **触发模式**

epoll 支持两种事件触发模式，分别是**边缘触发（edge-triggered，ET）**和**水平触发（level-triggered，LT）**。

- LT：当事件就绪后，会一直通知上层用户。epoll 默认使用该触发模式，表现为当一个文件描述符的事件就绪后，会一直出现在 `epoll_wait` 返回的就绪队列中。

- ET：只有事件发生变化时，才会通知上层用户。使用在给事件 `epoll_event` 结构体的 `events` 字段加上宏 `EPOLLET`，以读为例，表现为只有当新数据到来时，对应的就绪事件才会出现在 `epoll_wait` 返回的就绪队列中。

对于 LT，当内核通知文件描述符可读写时，接下来还可以继续去检测它的状态，看它是否依然可读或可写。所以在收到通知后，没必要一次执行尽可能多的读写操作。

但 ET 不能这样，如果一次不把数据都读完的话，并且不来新数据的话，就不会再通知我们进行 I/O，会导致数据一直留在缓冲区中。所以在收到通知后要尽可能多的读，以免错失读的机会，写同理。

所以 ET 通常要配合非阻塞 I/O 使用，我们进行一个 死循环，直到返回错误，并且 `errno` 为 `EAGAIN` 或 `EWOULDBLOCK`。

一般来说 ET 要比 LT 效率更高，原因如下：

- 边缘触发可以减少 epoll_wait 的系统调用次数。

- 边缘触发强制程序员一次性将缓冲区数据读完，

epoll的高性能, 是有一定的特定场景的. 如果场景选择的不适宜, epoll的性能可能适得其反。对于多连接, 且多连接中只有一部分连接比较活跃时, 比较适合使用epoll。例如, 典型的一个需要处理上万个客户端的服务器, 例如各种互联网APP的入口服务器, 这样的服务器就很适合epoll。如果只是系统内部, 服务器和服务器之间进行通信, 只有少数的几个连接, 这种情况下用epoll就并不合适. 具体要根据需求和场景特点来决定使用哪种IO模型。


### **惊群现象**

[Linux惊群效应详解（最详细的了吧）](https://blog.csdn.net/weixin_41282397/article/details/87608527?ops_request_misc=%257B%2522request%255Fid%2522%253A%2522172060260616800207033707%2522%252C%2522scm%2522%253A%252220140713.130102334..%2522%257D&request_id=172060260616800207033707&biz_id=0&utm_medium=distribute.pc_search_result.none-task-blog-2~all~sobaiduend~default-1-87608527-null-null.142^v100^pc_search_result_base9&utm_term=%E6%83%8A%E7%BE%A4%E6%95%88%E5%BA%94&spm=1018.2226.3001.4187){target="_blank"}


## **Reactor**

Reactor 即反应堆，是基于事件驱动的设计模式，拥有一个或多个并发输入源，有一个服务处理器和多个请求处理器，服务处理器会同步的将输入的请求事件以多路复用的方式分发给相应的请求处理器。

