
## **简介**

gcc（GNU Compiler Collection，GNU编译器套件）是由GNU开发的编程语言编译器。GNU编译器套件包括C、C++、 Objective-C、 Fortran、Java、Ada 和 Go 语言前端，也包括了这些语言的库（如libstdc++，libgcj等。）

g++ 是 gcc 内置的用于编译 C++ 程序的编译器，用法与 gcc 相同。

## **使用**

### **基础用法**

对于 gcc/g++ 一些常见的选项就直接列出：

|选项|作用|
|:-:|:-:|
-o [文件名]          |选择结果输出到的文件
-E                  |只做预处理，生成 .i 文件
-S                  |只完成到汇编，生成 .s 汇编文件
-c                  |不做连接，生成 .o 可重定向二进制文件
-fPIC               |生成与位置无关码，用于制作动态库
-std=[C/C++标准号]   |选择编译标准   
-O[0/1/2/3]         |选择优化等级


### **Sanitizer**

[Sanitizers](https://github.com/google/sanitizers){target="_blank"} 是谷歌发起的开源工具集，包括AddressSanitizer，MemorySanitizer, ThreadSanitizer, LeakSanitizer, Sanitizers项目本身是llvm项目的一部分，gcc自带的工具， gcc从4.8版本开始支持Address和Thread Sanitizer，4.9版本开始支持Leak Sanitizer和UBSanitizer。



#### **AddressSanitizer**

AddressSanitizer 是一个内存检测工具，可以用于检测内存泄漏，内存越界，栈溢出等问题。


```cpp
#include <stdlib.h>
int main() 
{
  char *x = (char*)malloc(10 * sizeof(char*));
  free(x);
  return x[5];
}
```

