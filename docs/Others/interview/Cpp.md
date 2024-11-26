


## **static**

1. 修饰局部变量，改变其生命周期，与全局变量相同

2. 修饰全局变量和函数，改变其链接属性，外链接变为内链接

## **大小端**

就看低地址，低地址存高位就是大端，存低位就是小段。


## **内存对齐**

对齐规则：

- 每个成员的开始位置必须在对齐数的整数倍位置处

- 各个成员的对齐数是默认对齐数和成员变量大小取小。默认对齐数一般为 8

- 整个结构体大小为所有对齐数中最大值的整数倍

可以通过预编译指令改变默认对齐数：

```cpp
#pragma pack(4) // 默认对齐数改为 4
```

## **位段、联合体，枚举**


位段的成员必须是整形变量，后面加一个冒号和数字代表成员占用的字节数

```cpp
struct A {
	int a : 2;
	long long b : 6;
};
```

联合体中所有成员公用一块空间，所以联合体大小为所有成员中的最大值。


```cpp
union A {
    int a;
    char b;
};
```

枚举如下：

```cpp
enum A {
	aa,
	bb = 2,
};
```

## **指针常量和常量指针**

```cpp
// 指针常量：指向常量的指针
const int* p1;
int const* p1; 
// 常量指针 ：指针是一个常量，不能改变指向位置
int* const p1; 
```


## **malloc 原理**

1. 当申请的空间小于 128KB 时会先去空闲链表管理的空闲内存中查看,若空闲的内存能满足所需大小,则直接分配该内存.若空闲内存不足,则需要调用系统调用扩大空间来满足需求

2. 当申请的空间大于 128KB 时,会直接调用系统调用来开辟空间

3. 调用malloc时，只是分配了对应的虚拟地址空间。只有当访问该部分内存时才会真正分配物理内存并将物理内存和虚拟内存建立映射关系，并且根据实际使用多少内存分配多少物理内存


## **封装**

封装就是将数据和处理数据的方法统一进行整合，并且将对象的属性和实现细节给隐藏了起来, 仅仅对外公开部分接口来和对象进行交互。

## **this 指针**

this 指针是成员函数的参数，会有编译器隐式的传参，所以 this 指针是在栈上存储的。并且 this 指针可以为空，只要在成员函数中没有访问过 this 指针就行，只要访问过就会运行错误。


## **前后置++**

后置 ++ 要有一个 int 类型的参数。

## **`explicit`**

修饰单参数的构造函数、转换函数（C++11）和推到指引（C++17），避免单参数的隐式类型转换，但不能修饰复制初始化和隐式类型转换


## **内存管理**

### **new 和 malloc 的区别**

使用上的区别：

- new 是操作符，而 malloc 是函数

- new 传递类型和元素个数，malloc 显示传入要申请的空间大小

- malloc 成功返回 void* 的指针需要强转，new 不需要

- malloc 失败返回 nullptr ，new 抛异常

实现上的区别：

- new/delete 会调用构造/析构，malloc/free 不会

- new 底层就是调用 operator new 全局函数申请空间，operator new 底层就是 malloc


### **内存泄漏**

一些原因导致申请到的资源得不到释放，随着程序的运行，可用资源越来越少。这个资源可能是：内存，文件描述符等等。

现代 C++ 项目中不会显示调用 new ，都是使用智能指针和 `make_shared、make_unique` 来申请资源。


## **泛型编程**

也就是 CPP 元编程，所谓元编程就是编写一些指令或代码，让计算机（编译器）去生成具体的代码。

这里 Cpp 元就是编写与类型无关的模板函数和模板类，让编译器去生成具体类型的代码。在 C++STL 库中大量的使用了 Cpp 元编程的技巧。元编程编写的程序编译时间较长，但通常有较高的运行效率。


## **vertor 扩容**

VS下选择1.5倍的方式扩容, g++下选择2倍的方式扩容。


