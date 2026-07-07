# Paint 函数

Paint 对象定义"要绘制的内容"——可以是纯色、渐变或纹理。Paint 对象是独立的，可以创建一次并在多个绘制操作或上下文中重用。使用完毕后调用 `cg_paint_destroy` 释放。

## 纯色 Paint

### cg_paint_create_rgb

```c
struct cg_paint_t * cg_paint_create_rgb(float r, float g, float b);
```

创建不透明度为 `1.0` 的纯色 paint。等价于 `cg_paint_create_rgba(r, g, b, 1.0f)`。

### cg_paint_create_rgba

```c
struct cg_paint_t * cg_paint_create_rgba(float r, float g, float b, float a);
```

创建纯色 paint。所有分量被钳制到 `[0.0, 1.0]`。

### cg_paint_create_color

```c
struct cg_paint_t * cg_paint_create_color(struct cg_color_t * color);
```

从 `cg_color_t` 结构体创建纯色 paint。等价于 `cg_paint_create_rgba(color->r, color->g, color->b, color->a)`。

## 渐变 Paint

### cg_paint_create_linear_gradient

```c
struct cg_paint_t * cg_paint_create_linear_gradient(
    float x1, float y1,
    float x2, float y2,
    enum cg_spread_method_t spread,
    struct cg_gradient_stop_t * stops,
    int nstops,
    struct cg_matrix_t * m);
```

创建线性渐变 paint。渐变从 `(x1, y1)` 处的 `stops[0].color` 过渡到 `(x2, y2)` 处的 `stops[nstops-1].color`。中间色标在这些端点之间插值。

| 参数        | 描述                                               |
|------------|----------------------------------------------------|
| `x1, y1`   | 渐变线的起点                                        |
| `x2, y2`   | 渐变线的终点                                        |
| `spread`   | 如何处理渐变方向上 `[0, 1]` 之外的坐标                |
| `stops`    | `nstops` 个颜色色标数组（偏移量会被钳制和排序）        |
| `m`        | 可选的用户空间到渐变空间的变换矩阵，`NULL` 表示单位矩阵  |

### cg_paint_create_radial_gradient

```c
struct cg_paint_t * cg_paint_create_radial_gradient(
    float cx0, float cy0, float r0,
    float cx1, float cy1, float r1,
    enum cg_spread_method_t spread,
    struct cg_gradient_stop_t * stops,
    int nstops,
    struct cg_matrix_t * m);
```

创建径向渐变 paint，从偏移量 `0` 处的圆 `(cx0, cy0, r0)` 过渡到偏移量 `1` 处的圆 `(cx1, cy1, r1)`。

:::note
paint 中的 `values` 数组存储为 `{cx0, cy0, r0, cx1, cy1, r1}`（先起始圆，再结束圆）。
:::

## 纹理 Paint

### cg_paint_create_texture

```c
struct cg_paint_t * cg_paint_create_texture(
    struct cg_surface_t * surface,
    enum cg_texture_type_t type,
    float opacity,
    struct cg_matrix_t * m);
```

创建纹理 paint。surface 会被引用（引用计数增加），当 paint 销毁时释放。`opacity` 被钳制到 `[0, 1]`，并与上下文的全局不透明度相乘。

可选矩阵 `m`（若为 `NULL` 则使用单位矩阵）定义用户空间到纹理空间的变换。

## 生命周期

### cg_paint_destroy

```c
void cg_paint_destroy(struct cg_paint_t * paint);
```

减少引用计数。归零时：
- 对于纹理 paint：对引用的 surface 调用 `cg_surface_destroy`。
- 对于所有 paint：释放内存。

### cg_paint_reference

```c
struct cg_paint_t * cg_paint_reference(struct cg_paint_t * paint);
```

增加引用计数并返回 paint。传入 `NULL` 安全。
