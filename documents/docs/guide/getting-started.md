# 快速开始

本节通过一个简单示例，帮助你在几分钟内上手 libcg。

## 前置准备

1. 已按[编译安装](./build-guide)完成 libcg 的编译；
2. 一个支持 C99 的编译器（gcc / clang）。

## 第一步：创建 Surface 和 Context

所有绘图操作都需要一个 surface（像素画布）和 context（绘图上下文）：

```c
#include <cg.h>

struct cg_surface_t * surface = cg_surface_create(256, 256);
struct cg_ctx_t * ctx = cg_create(surface);
```

## 第二步：绘制图形

绘制一个红色填充的矩形：

```c
cg_set_source_rgba(ctx, 1.0, 0.0, 0.0, 1.0);
cg_rectangle(ctx, 50, 50, 156, 156);
cg_fill(ctx);
```

## 第三步：描边

再绘制一个蓝色边框的矩形：

```c
cg_set_source_rgba(ctx, 0.0, 0.0, 1.0, 1.0);
cg_set_line_width(ctx, 3.0);
cg_rectangle(ctx, 50, 50, 156, 156);
cg_stroke(ctx);
```

## 第四步：销毁资源

绘制完成后释放上下文和 surface：

```c
cg_destroy(ctx);
cg_surface_destroy(surface);
```

## 完整示例

```c
#include <cg.h>

int main(void)
{
    struct cg_surface_t * surface = cg_surface_create(256, 256);
    struct cg_ctx_t * ctx = cg_create(surface);

    /* 红色填充 */
    cg_set_source_rgba(ctx, 1.0, 0.0, 0.0, 1.0);
    cg_rectangle(ctx, 50, 50, 156, 156);
    cg_fill(ctx);

    /* 蓝色描边 */
    cg_set_source_rgba(ctx, 0.0, 0.0, 1.0, 1.0);
    cg_set_line_width(ctx, 3.0);
    cg_rectangle(ctx, 50, 50, 156, 156);
    cg_stroke(ctx);

    /* 读取 surface->pixels 获取 RGBA32 像素数据 */
    /* unsigned char * pixels = surface->pixels; */

    cg_destroy(ctx);
    cg_surface_destroy(surface);
    return 0;
}
```

## 更多操作

- [API 参考](../api/architecture)：了解所有可用函数的详细说明；
- [使用示例](../examples/basic-shapes)：查看各类绘图场景的完整代码；
- [参考列表](../reference/enumerations)：查看枚举类型与默认值速查表。
