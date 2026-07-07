# 编译安装

本文档说明如何从源码编译 libcg 库及示例程序。

## 依赖

libcg 仅依赖标准 C 库（libc 和 libm），无需安装额外的第三方库。编译示例程序需要一个支持 C99 的编译器（如 gcc 或 clang）。

## 编译库与示例

在源码根目录执行：

```shell
cd libcg
make
```

编译完成后将生成：

- `src/libcg.a` — 静态库
- `examples/examples` — 示例程序（运行后在 examples 目录下生成 PNG 图片）

## 仅编译库

```shell
make -C src
```

生成 `src/libcg.a` 静态库及 `src/cg.o`、`src/xft.o` 目标文件。

## 仅编译示例

```shell
make -C examples
```

示例程序链接 `src/libcg.a`，运行后会在当前目录输出多张 PNG 图片，展示 libcg 的各种绘图能力。

## 交叉编译

通过 `CROSS_COMPILE` 变量指定交叉编译工具链前缀：

```shell
make CROSS_COMPILE=arm-linux-gnueabihf-
```

## 集成到项目

将以下文件复制到你的项目中即可使用：

- `src/cg.h` — 主头文件
- `src/cg.c` — 实现文件
- `src/xft.h` — FreeType 轮廓处理头文件
- `src/xft.c` — FreeType 轮廓处理实现

在源码中包含头文件并链接数学库：

```c
#include <cg.h>
```

编译时链接 `-lm`：

```shell
gcc -I src my_program.c src/cg.c src/xft.c -lm -o my_program
```

## 清理

```shell
make clean
```

清除所有编译生成的目标文件、静态库和示例程序。
