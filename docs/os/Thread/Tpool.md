

## **简介**

线程池（thread pool）: 一种线程使用模式，线程过多会带来调度开销，进而影响缓存局部性和整体性能。而线程池维护着多个线程，等待着监督管理者分配可并发执行的任务。这避免了在短时间任务创建与销毁线程的代价。线程池不仅能够保证内核的充分利用，还能防止过分调度。可用线程数据取决于可用的并发处理器、处理器内核、内存、网络sockets等数量。


## **实现**

实现一个简单的懒汉单例模式的线程池。

这里线程池内部是一个线程数组 `std::vector<std::thread>` 用做管理线程和一个自己实现的循环队列 `::ring_queue<::thread_task>` 做任务队列。

简单封装了一个 `thread_task` 用来维护线程执行的任务，其内部维护一个可调用对象及其参数。

线程池提供添加任务和启动的功能。在启动后，空闲线程会自动从任务队列中取任务并执行，直到任务队列为空，则停止。

???+ note "线程池"

    === "threadpool.hpp"

        ```cpp
        #pragma once
        #include <unistd.h>
        #include <thread>
        #include <vector>
        #include <iostream>
        #include <functional>
        #include <tuple>
        #include "ring_queue.hpp"

        // 生成 L ~ (R - 1) 的 元组索引
        template<std::size_t ...N>
        class tuple_indices {};

        template<std::size_t L,std::size_t R,std::size_t ..._Ind>
        class make_indices : public make_indices<L,R - 1,R - 1,_Ind...> {};

        template<std::size_t L,std::size_t ..._Ind>
        class make_indices<L,L,_Ind...> : public tuple_indices<_Ind...> {};


        // 编译期求元组大小
        template<class Tp>
        struct tuple_size{ };

        template<class ...Args>
        struct tuple_size<std::tuple<Args...>>
        {
            // 编译期求值表达式
            static constexpr std::size_t value = sizeof...(Args);
        };


        /**
        * @brief 线程任务，包装任意可调用对象及其参数，禁止拷贝，只能移动。
        *        只是简单的封装，不能获得函数执行返回值，不支持引用语义。
        */
        class thread_task
        {
            // 对回调函数的包装，用于处理任意类型的可调用对象机器参数，并在需要时执行他们
            struct _m_invoke_impl_base
            { virtual void run() = 0; };

            using self = thread_task;
            using _invoker_ptr = std::unique_ptr<_m_invoke_impl_base>;


            template<class _Tuple>
            struct _m_invoke_impl : public _m_invoke_impl_base
            {   
                _Tuple __m_t;   // 存可调用对象和参数包的元组

                template<class Callable,class ...Args>
                _m_invoke_impl(Callable&& __f,Args&&... __args)
                    :__m_t(
                        std::tuple<Callable,Args...>(
                            __f,std::forward<Args>(__args)...
                        )
                    )
                { }

                // 调用 __f(__args...)
                template<size_t ..._Ind>
                void _m_invoke(tuple_indices<_Ind...>)
                {
                    std::get<0>(__m_t)(std::get<_Ind>(std::move(__m_t))...);
                }

                // 生成 tuple 索引包，并调用函数
                virtual void run()
                { 
                    _m_invoke(make_indices<
                        1, tuple_size<decltype(__m_t)>::value /*1 ~ (N - 1) 的索引包*/
                    >()); 
                }
            };

            _invoker_ptr _m_impl;   // 指向具体的任务对象（基类指针指向子类对象实现多态）


        public:
            thread_task() = default;

            template<class Callable,class ...Args>
            thread_task(Callable&& func,Args&&... args)
                :_m_impl(
                    new _m_invoke_impl<std::tuple<Callable,Args...>>(
                        func,std::forward<Args>(args)...
                    )
                )
            { }

            thread_task(const self&) = delete;
            self& operator=(const self&) = delete;

            thread_task(self&&) = default;
            self& operator=(self&&) = default;

            // 执行任务
            void run()
            { _m_impl->run(); }
        };


        /**
        * @brief 懒汉单例模式的线程池
        */
        class thread_pool
        {
        private:
            std::vector<std::thread> _pool; // 线程池
            ::ring_queue<thread_task> _task_que; // 任务队列

            // 实现线程安全的单例模式
            static std::mutex _single_mt;
            static thread_pool* _single;

            void _run()
            {
                for(;;)
                { _task_que.get().run(); /* 从线程队列区取任务 */ }
            }

            thread_pool(size_t thread_pool_size,size_t task_que_size)
                :_pool(thread_pool_size)
                ,_task_que(task_que_size)
            { }

        public:

            // 单例模式
            thread_pool(const thread_pool&) = delete;
            thread_pool& operator=(const thread_pool&) = delete;

            // 创建单例线程池，要保证线程安全
            static thread_pool* 
                get_pool(size_t thread_pool_size,size_t task_que_size) noexcept
            {
                // 如果以及创建成功，反复申请锁效率低下，这里直接返回
                if(_single != nullptr) return _single;
                std::unique_lock<std::mutex> lg(_single_mt);
                // 保证第一次创建线程池时，只有一个线程成功申请
                if(_single != nullptr) return _single;
                return _single = new thread_pool(thread_pool_size,task_que_size);
            }

            /**
            * @brief 启动线程池
            */
            void start() noexcept
            {
                for(auto&td:_pool)
                    td = std::thread(std::bind(&thread_pool::_run,this));
            }

            /**
            * @brief 向线程池任务队列中加入新任务
            * @param new_task 新的任务
            */
            void add_task(::thread_task&& new_task)
            {
                _task_que.put(std::move(new_task));
            }

            /**
            * @brief 通过可调用对象及其参数构造新的线程任务
            * @param _func 可调用对象
            * @param ..._args 参数
            */
            template<class Callable,class ...Args>
            void add_task(Callable&& _func,Args&&... _args)
            {
                _task_que.put(::thread_task(_func,std::forward<Args>(_args)...));
            }
        };


        thread_pool* thread_pool::_single = nullptr;
        std::mutex thread_pool::_single_mt;



        ```

    === "ring_queue.hpp"

        ```cpp
        #pragma once
        #include <iostream>
        #include <vector>
        #include <thread>
        #include <pthread.h>
        #include <semaphore.h>
        #include <mutex>

        /**
        *  @brief 线程安全的循环队列。
        */
        template<class Tp>
        class ring_queue
        {
            std::vector<Tp> _rq;  // 数组实现循环队列
            int _p_pos;     // 生产者当前可以放数据的位置
            int _c_pos;     // 消费者当前可以取的数据位置
            int _capcity;   // 队列容量 
            std::mutex _p_mt;   // 实现生产者间的互斥
            std::mutex _c_mt;   // 实现消费者间的互斥
            sem_t _p_sem;   // 剩余空间
            sem_t _c_sem;   // 队列中的数据量

            int P(sem_t& _sem) 
            { return sem_wait(&_sem); }

            int V(sem_t& _sem)
            { return sem_post(&_sem); }
            
        public:
            ring_queue(int n)
                :_rq(n)
                ,_p_pos(0)
                ,_c_pos(0)
            {
                sem_init(&_p_sem,0,n);
                sem_init(&_c_sem,0,0);
                _capcity = n;
            }

            ring_queue(const ring_queue&) = delete;

            void put(const Tp& in)
            {
                P(_p_sem); // 先申请空间
                _p_mt.lock();   // 放数据时加锁
                _rq[_p_pos] = in;
                (_p_pos += 1) %= _capcity;
                _p_mt.unlock();
                V(_c_sem);
            }

            void put(Tp&& in)
            {
                P(_p_sem); // 先申请空间
                _p_mt.lock();   // 放数据时加锁
                _rq[_p_pos] = std::move(in);
                (_p_pos += 1) %= _capcity;
                _p_mt.unlock();
                V(_c_sem);
            }

            Tp get() 
            {
                P(_c_sem); // 先申请数据
                _c_mt.lock();   // 取数据时加锁
                Tp ret = std::move(_rq[_c_pos]);
                (_c_pos += 1) %= _capcity;
                _c_mt.unlock(); 
                V(_p_sem);
                return ret;
            }

            ~ring_queue()
            {
                sem_destroy(&_c_sem);
                sem_destroy(&_p_sem);
            }
        };
        ```