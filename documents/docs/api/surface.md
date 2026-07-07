# Surface 函数

Surface 是像素画布，存储预乘 RGBA32 格式的像素数据。所有绘图操作最终都渲染到 surface 上。

## cg_surface_create

```c
struct cg_surface_t * cg_surface_create(int width, int height);
```

分配给定大小的新 surface。像素缓冲区初始化为零（完全透明的黑色）。`owndata` 设为 `1`。

## cg_surface_create_for_data

```c
struct cg_surface_t * cg_surface_create_for_data(int width, int height, void * pixels);
```

创建包装已有像素缓冲区的 surface。`pixels` 必须至少为 `width * height * 4` 字节，格式为预乘 RGBA32。`owndata` 设为 `0`，意味着调用 `cg_surface_destroy` 将**不会**释放像素数据——调用者负责其生命周期。

## cg_surface_destroy

```c
void cg_surface_destroy(struct cg_surface_t * surface);
```

减少引用计数。当计数归零时，释放像素缓冲区（若 `owndata == 1`）和 surface 结构体。传入 `NULL` 安全。

## cg_surface_reference

```c
struct cg_surface_t * cg_surface_reference(struct cg_surface_t * surface);
```

增加引用计数并返回 surface。传入 `NULL` 安全（返回 `NULL`）。

## 像素格式

Surface 的像素以**预乘 RGBA32** 格式存储（行优先），每个像素 4 字节：

```
[偏移]  字节      含义
0       R'        预乘后的红色分量
1       G'        预乘后的绿色分量
2       B'        预乘后的蓝色分量
3       A         Alpha 透明度
```

其中 `R' = R * A / 255`，`G' = G * A / 255`，`B' = B * A / 255`。

`stride` 等于 `width * 4`，可通过 `surface->stride` 获取。像素数据指针通过 `surface->pixels` 获取。

## 使用示例

```c
/* 创建 256x256 的 surface */
struct cg_surface_t * surface = cg_surface_create(256, 256);

/* 绘图操作... */

/* 访问像素数据 */
unsigned char * pixels = surface->pixels;
int width = surface->width;
int height = surface->height;
int stride = surface->stride;

/* 释放 */
cg_surface_destroy(surface);
```
