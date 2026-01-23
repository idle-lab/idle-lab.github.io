## 协程

C++ 20 协程是一个无栈（stackless）的协程，同时，C++ 20 协程是非对称的协程

**“无栈”**这个名字有点容易让人误解。它不是说协程没有调用栈，而是指协程本身的状态和数据不存储在传统的执行栈（execution stack）上。一个传统的、有栈的协程（Stackful Coroutine），比如 Boost.Context 或 Go 语言的 goroutine，会为自己分配一块完整、独立的内存空间作为调用栈。这个栈足够大，可以让你在这个协程里像写普通函数一样嵌套调用多层函数。每个协程都有自己的栈，互不干扰。

而 C++20 的无栈协程，它共享调用它的线程的栈。它的“状态”被存储在了堆（heap）上的一个独立对象（称为“协程状态/帧”，coroutine state/frame）里。

**“非对称”**指的是协程之间的调用/控制流关系是不对称的。协程之间存在一个明显的调用者和被调用者（resumer 和 resumee）的层级关系。

一个协程（Callee）只能将控制流返回（`co_await` 或 `co_return`） 到恢复（`resume`）它的那个协程（`Caller`）。你不能随意将控制权转移到某个任意其他的协程。

## 实现

C++的协程（协程函数）内部可以用 `co_await` , `co_yield` 两个关键字挂起协程，`co_return` 关键字进行返回，当一个函数内部出现这三个关键字的任意一个，这个函数就会被编译器识别为协程，编译器会为协程函数做很多额外的工作，下面我们一一介绍。

### **coroutine_handle**

编译器为每一个协程提供了 `std::coroutine_handle` 工具用于管理协程，例如：创建协程、销毁协程、恢复协程、判断协程状态等等。

??? "std::coroutine_handle 源码"
    ```cpp
    _EXPORT_STD template <class _CoroPromise>
    struct coroutine_handle {
        constexpr coroutine_handle() noexcept = default;
        constexpr coroutine_handle(nullptr_t) noexcept {}

        _NODISCARD static coroutine_handle from_promise(_CoroPromise& _Prom) noexcept { // strengthened
            const auto _Prom_ptr  = const_cast<void*>(static_cast<const volatile void*>(_STD addressof(_Prom)));
            const auto _Frame_ptr = __builtin_coro_promise(_Prom_ptr, 0, true);
            coroutine_handle _Result;
            _Result._Ptr = _Frame_ptr;
            return _Result;
        }

        coroutine_handle& operator=(nullptr_t) noexcept {
            _Ptr = nullptr;
            return *this;
        }

        _NODISCARD constexpr void* address() const noexcept {
            return _Ptr;
        }

        _NODISCARD static constexpr coroutine_handle from_address(void* const _Addr) noexcept { // strengthened
            coroutine_handle _Result;
            _Result._Ptr = _Addr;
            return _Result;
        }

        constexpr operator coroutine_handle<>() const noexcept {
            return coroutine_handle<>::from_address(_Ptr);
        }

        constexpr explicit operator bool() const noexcept {
            return _Ptr != nullptr;
        }

        _NODISCARD bool done() const noexcept { // strengthened
            return __builtin_coro_done(_Ptr);
        }

        void operator()() const {
            __builtin_coro_resume(_Ptr);
        }

        void resume() const {
            __builtin_coro_resume(_Ptr);
        }

        void destroy() const noexcept { // strengthened
            __builtin_coro_destroy(_Ptr);
        }

        _NODISCARD _CoroPromise& promise() const noexcept { // strengthened
            return *reinterpret_cast<_CoroPromise*>(__builtin_coro_promise(_Ptr, 0, false));
        }

    private:
        void* _Ptr = nullptr;
    };
    ```

在创建协程时，`std::coroutine_handle` 会申请一块堆空间，保存协程所需的所有状态，在协程销毁时这块空间也需要被释放，你可以选择手动释放或者在协程结束时自动释放，这个后文介绍。


### **promise_type**

编译器要求协程的返回对象必须是一个协程管理对象，该对象必须包含 `promise_type` 这个子类。`promise_type` 用于管理协程执行逻辑，可以参考如下例子：


```cpp
class Coro {
public:
    struct promise_type {
        // 创建协程操作管理对象
        Coro get_return_object() {
            return Coro(std::coroutine_handle<promise_type>::from_promise(*this));
        }

        // 决定协程创建后是否马上执行
        // std::suspend_never：不挂起
        // std::suspend_always：挂起
        auto initial_suspend() noexcept {
            return std::suspend_always{};
        }

        // 协程执行完后是否马上销毁
        // std::suspend_never：不挂起，立即销毁资源
        // std::suspend_always：挂起，不立即销毁，等待外部 destroy 释放资源
        auto final_suspend() noexcept {
            return std::suspend_never{};
        }

        // 协程中有未处理的异常时如何处理
        void unhandled_exception() {
            std::cout << "未处理的异常" << std::endl;
        }

        // co_return 没有没有返回值时会调用该函数
        void return_void() {}
    };

    std::coroutine_handle<promise_type> handle;

    Coro(std::coroutine_handle<promise_type> handle) {
        this->handle = handle;
    }

    void resume() {
        if (!handle.done()) {
            handle.resume();
        }
    }
};

Coro MyCoro() {
    // some operation
}

int main() {
    auto co = MyCoro();

    // 手动执行协程
    co.resume();
}
```

在上面这个创建协程的例子中，会执行如下流程：

1. 创建 `promise_type` 对象

2. 用 `promise_type::get_return_object()` 函数返回协程管理对象

3. 调用 `promise_type::initial_suspend()` 函数确定协程初始化后是否马上执行

4. 协程中抛出了未处理的异常，`promise_type::unhandled_exception()` 函数会被执行处理异常

5. 协程结束之后，调用 `promise_type::final_suspend()` 函数来确定马上销毁协程，还是用户手动销毁

### **co_return**

co_return 可以直接结束协程逻辑、或者返回一个值再结束协程逻辑，当协程逻辑结束后，会自动调用 promise_type::final_suspend 函数 。

当 co_return 没有返回值时，编译器会调用 `void promise_type::return_void()` 函数来处理返回逻辑；当 co_return 有返回值时，它会将返回值传递到 `void promise_type::return_value(T value)` 函数中处理。


### **co_yield**

与 co_return 直接终止协程执行不同，co_yield 并不会结束协程，而是先向调用方生成一个中间值，再将自身挂起。它无需依赖任何外部事件，完全由协程主动触发挂起，后续需等待调用方显式恢复才能继续执行。

具体的执行流程，当协程执行到 co_yield value; 时：会调用 `auto promise_type::yield_value(T value)` 函数，其返回值决定协程在 co_yield 后是否立即挂起：

??? "example"

    ```cpp
    class Coro {
    public:
        struct promise_type {
            // 创建协程操作管理对象
            Coro get_return_object() {
                return Coro(std::coroutine_handle<promise_type>::from_promise(*this));
            }

            // 决定协程创建后是否马上执行
            // std::suspend_never：不挂起
            // std::suspend_always：挂起
            auto initial_suspend() noexcept {
                return std::suspend_always{};
            }

            // 协程执行完后是否马上销毁
            // std::suspend_never：不挂起，立即销毁资源
            // std::suspend_always：挂起，不立即销毁，等待外部 destroy 释放资源
            auto final_suspend() noexcept {
                return std::suspend_never{};
            }

            // 协程中有未处理的异常时如何处理
            void unhandled_exception() {
                std::cout << "未处理的异常" << std::endl;
            }

            // co_return 没有没有返回值时会调用该函数
            void return_void() {}

            auto yield_value(int value) {
                value_ = value;
                return std::suspend_always{};
            }

            int value_;
        };

        std::coroutine_handle<promise_type> handle;

        Coro(std::coroutine_handle<promise_type> handle) {
            this->handle = handle;
        }

        void resume() {
            if (!handle.done()) {
                handle.resume();
            }
        }
    };

    Coro MyCoro() {
        co_yield 10;
        co_yield 20;
        co_yield 30;
    }

    int main() {
        auto co = MyCoro();

        co.resume();
        std::cout << "第一次返回的值:" << co.handle.promise().value_ << std::endl;
        co.resume();
        std::cout << "第一次返回的值:" << co.handle.promise().value_ << std::endl;
        co.resume();
        std::cout << "第一次返回的值:" << co.handle.promise().value_ << std::endl;
    }
    ```

### **co_await**

co_await 需要根据条件进行挂起，挂起的时候，可能还需要执行某些操作，如网络/文件 IO。恢复的时候，也会执行某些逻辑。这也决定了 co_await 后面不能是简单的表达式，而是一个可等待（Awaitable）对象，它要求你的对象实现如下三个函数：

- `bool await_ready()`：在 co_await 时，首先执行这个函数，判断是否继续执行协程后续逻辑。true 表示继续执行，然后执行 `await_resume()` 返回结果；false 表示挂起，然后执行 `await_suspend()` 进行挂起前的操作。

- `void/bool/handle await_suspend(std::coroutine_handle<> handle)`：返回 void：执行后挂起；返回 bool：true 表示执行完后挂起，false 表示执行完后不挂起，继续执行 await_resume 函数；返回 handle：表示执行完后，继续恢复执行 handle 关联的协程。

- `T await_resume()`：协程恢复后执行，返回 co_await 的结果。


??? "example"

    ```cpp
    class IntReader {
    public:
        bool await_ready() const noexcept {
            return false;
        };

        void await_suspend(std::coroutine_handle<> h) noexcept {
            // 这里模拟耗时操作
            std::thread t([this, h] {
                sleep(1);
                value_ = 114514;
                h.resume();
            });

            t.detach();
        }

        int await_resume() {
            return value_;
        }

    private:
        int value_{0};
    };

    class Coro {
    public:
        struct promise_type {
            Coro get_return_object() {
                return Coro(std::coroutine_handle<promise_type>::from_promise(*this));
            }

            auto initial_suspend() noexcept {
                return std::suspend_always{};
            }

            auto final_suspend() noexcept {
                return std::suspend_never{};
            }

            void unhandled_exception() {
                std::cout << "未处理的异常" << std::endl;
            }

            void return_value(int value) {
                value_ = value;
            }

            int value_;
        };

        std::coroutine_handle<promise_type> handle;

        Coro(std::coroutine_handle<promise_type> handle) {
            this->handle = handle;
        }

        void resume() {
            if (!handle.done()) {
                handle.resume();
            }
        }
    };

    Coro MyCoro() {
        IntReader r;
        auto x = co_await r;

        std::cout << x << '\n';
    }
    ```

在 promise_type 中定义 `await_transform`，可以拦截 co_await 后的表达式，并将其转换为一个满足 awaitable 协议的对象，无论原始表达式本身是否可等待。


??? "await_transform"

    ```cpp
    #include <iostream>
    #include <coroutine>

    struct MyAwaiter
    {
        int value;
        bool await_ready() { return false; }
        void await_suspend(std::coroutine_handle<> handle) {}
        int await_resume() { return value; }
    };

    struct CoroManager
    {
        struct promise_type
        {
            int value;

            CoroManager get_return_object() { return CoroManager{ std::coroutine_handle<promise_type>::from_promise(*this) };}
            std::suspend_always initial_suspend() { return {}; }
            void return_void() {}
            std::suspend_always final_suspend() noexcept { return {}; }
            void unhandled_exception() {}

            auto await_transform(int val)
            {
                return MyAwaiter{ val };
            }
        };

        std::coroutine_handle<promise_type> handle;
        CoroManager(std::coroutine_handle<promise_type> handle)
        {
            this->handle = handle;
        }

        void resume() { handle.resume(); }
        ~CoroManager(){ handle.destroy(); }
    };

    CoroManager my_croutine()
    {
        int ret = co_await 100;
        std::cout << "ret = " << ret << std::endl;
    }

    int main()
    {
        CoroManager coro = my_croutine();
        coro.resume();
        coro.resume();

        return 0;
    }
    ```


----

参考：

[https://mengbaoliang.cn/archives/131970/#title-10](https://mengbaoliang.cn/archives/131970/#title-10){target="_blank"}

[bilibili](https://www.bilibili.com/video/BV1Cz9NYFE8E/?spm_id_from=333.337.search-card.all.click&vd_source=0de771c86d90f02a6cab8152f6aa173f){target="_blank"}