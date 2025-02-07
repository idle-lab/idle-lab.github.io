

## **简介**

CMake是个一个开源的跨平台自动化建构系统，用来管理软件建置的程序，并不依赖于某特定编译器，并可支持多层目录、多个应用程序与多个函数库。 

软件构建就是全自动完成代码编译、链接、打包的过程，以及管理不同组件、甚至第三方库的关联。

## **HolleWorld**

CMake 对项目的构建依赖于文件 CMakeLists.txt（大小写要完全一样），CMake 会通过该文件为我们生成对应平台下的原生工程文件（如：Linux 下的 Makefile，Visual Studio 工程的 .sln等）。

我们来看一个简单的 CMakeLists.txt 文件：

```cmake
cmake_minimum_required(VERSION 3.9) # 标识 CMake 最低版本

project(HelloWorld)     # 项目名称
set(CMAKE_CXX_STANDARD 11)  # C++ 标准版本
# 添加要生成的可执行文件，前一个是生成的可执行文件的名字，后面是依赖的 cpp 文件
add_executable(HelloWorld main.cpp) 
```

- add_executable：定义工程会生成一个可执行程序

格式如下：

```cmake
add_executable(可执行程序名 源文件名称)
```

源文件名可以是一个也可以是多个，如有多个可用空格或;间隔

```cmake
# 样式1
add_executable(app add.c div.c main.c mult.c sub.c)
# 样式2
add_executable(app add.c;div.c;main.c;mult.c;sub.c)
```

## **定义变量**

在上面的例子中一共提供了5个源文件，假设这五个源文件需要反复被使用，每次都直接将它们的名字写出来确实是很麻烦，此时我们就需要定义一个变量，将文件名对应的字符串存储起来，在cmake里定义变量需要使用set。

```cmake
# SET 指令的语法是：
# [] 中的参数为可选项, 如不需要可以不写
SET(VAR [VALUE] [CACHE TYPE DOCSTRING [FORCE]])
```

使用变量的格式如下：

```cmake
# 方式1: 各个源文件之间使用空格间隔
# set(SRC_LIST add.c  div.c   main.c  mult.c  sub.c)

# 方式2: 各个源文件之间使用分号 ; 间隔
set(SRC_LIST add.c;div.c;main.c;mult.c;sub.c)
add_executable(app  ${SRC_LIST})
```

### **指定 C++ 标准**

```cmake
#增加-std=c++11
set(CMAKE_CXX_STANDARD 11)
#增加-std=c++14
set(CMAKE_CXX_STANDARD 14)
#增加-std=c++17
set(CMAKE_CXX_STANDARD 17)
```

也可以再执行 cmake 命令时加上命令行参数 `-DCMAKE_CXX_STANDARD=17` 来指定 C++ 标准

### **指定输出路径**

cmake 中有一个宏叫做：`EXECUTABLE_OUTPUT_PATH`，用来指定可执行程序的输出路径 

```cmake
set(HOME /home/robin/Linux/Sort)
set(EXECUTABLE_OUTPUT_PATH ${HOME}/bin)
```

由于可执行程序是基于 cmake 命令生成的 makefile 文件然后再执行 make 命令得到的，所以如果此处指定可执行程序生成路径的时候使用的是相对路径 ./xxx/xxx，那么这个路径中的 ./ 对应的就是 makefile 文件所在的那个目录。


## **搜索文件**

在 CMake 中使用 `aux_source_directory` 命令可以查找某个路径下的所有源文件，命令格式为：

```CMAKE
aux_source_directory(<dir> <variable>)
```

`dir`：要搜索的目录

`variable`：将从 `dir` 目录下搜索到的源文件列表存储到该变量中

还有一个搜索文件的命令 `file`

```cmake
file(GLOB/GLOB_RECURSE 变量名 通配符表达式)
```

`GLOB`: 将指定目录下搜索到的满足条件的所有文件名生成一个列表，并将其存储到变量中。
`GLOB_RECURSE`：递归搜索指定目录，将搜索到的满足条件的文件名生成一个列表，并将其存储到变量中。

```cmake
file(GLOB MAIN_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB MAIN_HEAD ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
```

`CMAKE_CURRENT_SOURCE_DIR` 宏表示当前访问的 CMakeLists.txt 文件所在的路径。

关于要搜索的文件路径和类型可加双引号，也可不加:

```cmake
file(GLOB MAIN_HEAD "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")
```

## **包含头文件**

在编译项目源文件的时候，很多时候都需要将源文件对应的头文件路径指定出来，这样才能保证在编译过程中编译器能够找到这些头文件，并顺利通过编译。在CMake中设置要包含的目录也很简单，通过一个命令就可以搞定了，他就是 `include_directories`：

```cmake
include_directories(headpath)
```

eg：

```cmake
cmake_minimum_required(VERSION 3.0)
project(CALC)
set(CMAKE_CXX_STANDARD 11)
set(HOME /home/robin/Linux/calc)
set(EXECUTABLE_OUTPUT_PATH ${HOME}/bin/)
include_directories(${PROJECT_SOURCE_DIR}/include)
file(GLOB SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_executable(app  ${SRC_LIST})
```
其中，第六行指定就是头文件的路径，PROJECT_SOURCE_DIR宏对应的值就是我们在使用cmake命令时，后面紧跟的目录，一般是工程的根目录。


## **制作动态库或静态库**

有些时候我们编写的源代码并不需要将他们编译生成可执行程序，而是生成一些静态库或动态库提供给第三方使用，下面来讲解在cmake中生成这两类库文件的方法。

`add_library` 命令用于定义一个库目标，其基本语法如下：

```cmake
add_library(<name> [STATIC | SHARED | MODULE]
            [EXCLUDE_FROM_ALL]
            source1 source2 ... sourceN)
```

- `<name>`：库的名称。

- 库类型：
  
STATIC：构建静态库（通常扩展名为 .a 或 .lib）。

SHARED：构建动态（共享）库（通常扩展名为 .so、.dll 或 .dylib）。

MODULE：构建加载模块（插件），不带链接信息。

- 如果不指定类型，CMake 会默认使用全局变量 `BUILD_SHARED_LIBS` 的值：

若 BUILD_SHARED_LIBS 为 ON，则默认构建动态库；

若为 OFF，则构建静态库。

### **静态库示例**

假设项目结构如下：

```css
MyProject/
├── CMakeLists.txt
├── include/
│   └── mylib.h
└── src/
    ├── foo.cpp
    └── bar.cpp
```

CMakeLists.txt 示例：

```cmake
cmake_minimum_required(VERSION 3.10)
project(MyStaticLibrary)

# 将源文件列表存入变量
set(SOURCES
    src/foo.cpp
    src/bar.cpp
)

# 指定构建静态库
add_library(mylib STATIC ${SOURCES})

# 为目标设置头文件搜索路径
target_include_directories(mylib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
```

此配置会生成一个静态库文件（Linux 下为 libmylib.a，Windows 下为 mylib.lib）。

### **动态库示例**

修改库类型为 SHARED，配置如下：

```cmake
cmake_minimum_required(VERSION 3.10)
project(MySharedLibrary)

set(SOURCES
    src/foo.cpp
    src/bar.cpp
)

# 指定构建动态库
add_library(mylib SHARED ${SOURCES})

target_include_directories(mylib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
```
构建后生成的库文件（Linux 下为 libmylib.so，Windows 下为 mylib.dll 以及对应的导入库 mylib.lib）为动态库。

### **使用全局变量 BUILD_SHARED_LIBS**

如果希望根据编译选项灵活控制库的类型，可以利用全局变量 BUILD_SHARED_LIBS。例如：

```cmake
cmake_minimum_required(VERSION 3.10)
project(MyLibrary)

set(SOURCES
    src/foo.cpp
    src/bar.cpp
)

# 没有明确指定库的类型时，CMake 会根据 BUILD_SHARED_LIBS 决定类型
add_library(mylib ${SOURCES})

target_include_directories(mylib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
```

在配置项目时，可以通过命令行指定：

- 构建动态库：
```bash
cmake -DBUILD_SHARED_LIBS=ON ..
```

- 构建静态库：
```bash
cmake -DBUILD_SHARED_LIBS=OFF ..
```
这种方式可以让你在同一个 CMakeLists.txt 文件中，根据编译时的选项灵活决定库的类型。

<hr>

参考：

[CMake 保姆级教程](https://subingwen.cn/cmake/CMake-primer/#2-1-1-%E5%85%B1%E5%A4%84%E4%B8%80%E5%AE%A4){target=_blank}

