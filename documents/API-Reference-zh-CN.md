# libcg API 参考文档

libcg 是一个轻量级的 2D 计算机图形 C 库，其绘图模型借鉴自 Cairo / HTML Canvas。它通过基于 span 的光栅化将抗锯齿图形渲染到 RGBA32 像素缓冲区中。

---

## 目录

1. [架构概述](#架构概述)
2. [数据类型](#数据类型)
    - [基础结构体](#基础结构体)
    - [枚举类型](#枚举类型)
    - [对象结构体](#对象结构体)
3. [矩阵函数](#矩阵函数)
4. [Surface 函数](#surface-函数)
5. [Paint 函数](#paint-函数)
6. [Path 函数](#path-函数)
7. [Context 函数](#context-函数)
    - [创建与销毁](#创建与销毁)
    - [状态管理](#状态管理)
    - [查询函数](#查询函数)
    - [空间查询](#空间查询)
    - [Paint 与样式](#paint-与样式)
    - [变换](#变换)
    - [路径构建](#路径构建)
    - [渲染](#渲染)
8. [完整示例](#完整示例)

---

## 架构概述

libcg 采用有状态的绘图模型。核心对象是 `cg_ctx_t`（绘图上下文），它持有：

- 对目标 `cg_surface_t` 像素缓冲区的引用
- 正在构建的当前 `cg_path_t`
- `cg_state_t` 对象的栈（通过 `cg_save` / `cg_restore` 压入/弹出）

每个状态保存：当前 paint（纯色、渐变或纹理）、变换矩阵、描边样式（线宽、线帽、连接、斜接限制、虚线模式）、填充规则、合成操作符、全局不透明度以及裁剪区域。

**内存管理**：Surface、Paint 和 Path 对象使用引用计数。接受这些对象的函数会在内部按需增加引用计数（例如 `cg_set_paint` / `cg_paint_create_texture` 内部调用 `cg_paint_reference` 和 `cg_surface_reference`）。每个 `_create` 必须配以对应的 `_destroy`。

**默认状态值**（创建上下文时）：

| 属性         | 默认值                        |
|-------------|------------------------------|
| Paint       | `NULL`（解释为 `state->color`） |
| 颜色         | `{0, 0, 0, 1}`（黑色）        |
| 矩阵         | 单位矩阵                      |
| 线宽         | `1.0`                        |
| 线帽         | `CG_LINE_CAP_BUTT`           |
| 线段连接     | `CG_LINE_JOIN_MITER`         |
| 斜接限制     | `10.0`                       |
| 填充规则     | `CG_FILL_RULE_NON_ZERO`      |
| 操作符       | `CG_OPERATOR_SRC_OVER`       |
| 不透明度     | `1.0`                        |

**颜色分量值**在整个 API 中均为 `[0.0, 1.0]` 范围内的浮点数。库内部会将值钳制到此范围。

**坐标系**：原点在左上角，X 向右递增，Y 向下递增。角度以弧度为单位，正方向为顺时针。

---

## 数据类型

### 基础结构体

#### cg_point_t

```c
struct cg_point_t {
    float x;
    float y;
};
```

二维空间中的一个点。

#### cg_rect_t

```c
struct cg_rect_t {
    float x;
    float y;
    float w;
    float h;
};
```

一个轴对齐矩形，锚定在左上角 `(x, y)`，宽度为 `w`，高度为 `h`。

#### cg_matrix_t

```c
struct cg_matrix_t {
    float a;  float b;
    float c;  float d;
    float tx; float ty;
};
```

一个 2D 仿射变换矩阵，将点 `(x, y)` 映射为：

```
x' = a*x + c*y + tx
y' = b*x + d*y + ty
```

齐次坐标形式：
```
| a  c  tx |
| b  d  ty |
| 0  0  1  |
```

#### cg_color_t

```c
struct cg_color_t {
    float r;
    float g;
    float b;
    float a;
};
```

RGBA 颜色。构建 paint 时所有分量会被钳制到 `[0.0, 1.0]`。

#### cg_gradient_stop_t

```c
struct cg_gradient_stop_t {
    float offset;
    struct cg_color_t color;
};
```

定义渐变中的一个位置-颜色对。`offset` 必须在 `[0.0, 1.0]` 范围内。偏移量会在内部被钳制和排序——如果某个色标的偏移量小于前一个色标，它会被提升到前一个色标的值。

---

### 枚举类型

#### cg_path_command_t

内部用于描述路径元素：

| 值                          | 点数 | 描述                      |
|-----------------------------|------|---------------------------|
| `CG_PATH_COMMAND_MOVE_TO`   | 1    | 移动到新点，开始一个子路径   |
| `CG_PATH_COMMAND_LINE_TO`   | 1    | 直线到目标点               |
| `CG_PATH_COMMAND_CUBIC_TO`  | 3    | 三次贝塞尔曲线（控制点1、控制点2、终点） |
| `CG_PATH_COMMAND_CLOSE`     | 1    | 闭合当前子路径             |

*注意：二次贝塞尔曲线在添加到路径时会内部转换为三次。*

#### cg_spread_method_t

控制渐变颜色在 `[0, 1]` 范围外如何采样：

| 值                          | 描述                      |
|-----------------------------|---------------------------|
| `CG_SPREAD_METHOD_PAD`      | 钳制到最近的边缘颜色        |
| `CG_SPREAD_METHOD_REFLECT`  | 在每个整数边界处镜像渐变     |
| `CG_SPREAD_METHOD_REPEAT`   | 重复渐变（环绕）            |

#### cg_gradient_type_t

| 值                          | 描述     |
|-----------------------------|---------|
| `CG_GRADIENT_TYPE_LINEAR`   | 线性渐变 |
| `CG_GRADIENT_TYPE_RADIAL`   | 径向渐变 |

#### cg_texture_type_t

| 值                         | 描述                                          |
|---------------------------|-----------------------------------------------|
| `CG_TEXTURE_TYPE_PLAIN`   | 单次绘制纹理（区域外为透明）                      |
| `CG_TEXTURE_TYPE_TILED`   | 纹理重复以填充整个绘制区域                        |

*注意：经过变换的非平铺纹理，以及带有旋转/剪切的平铺纹理，使用双线性过滤。其他组合使用最近邻采样。*

#### cg_line_cap_t

| 值                       | 描述                                |
|--------------------------|-------------------------------------|
| `CG_LINE_CAP_BUTT`       | 平头端点，恰好在端点处结束（默认）      |
| `CG_LINE_CAP_ROUND`      | 半圆形端点，超出端点延伸              |
| `CG_LINE_CAP_SQUARE`     | 方形端点，超出端点延伸半个线宽         |

#### cg_line_join_t

| 值                        | 描述                                  |
|---------------------------|---------------------------------------|
| `CG_LINE_JOIN_MITER`      | 尖角，受斜接限制裁剪（默认）             |
| `CG_LINE_JOIN_ROUND`      | 圆角                                  |
| `CG_LINE_JOIN_BEVEL`      | 平角（斜角）                           |

#### cg_fill_rule_t

确定自相交路径中哪些区域属于"内部"：

| 值                          | 描述                    |
|-----------------------------|------------------------|
| `CG_FILL_RULE_NON_ZERO`     | 非零环绕规则（默认）      |
| `CG_FILL_RULE_EVEN_ODD`     | 奇偶规则                |

#### cg_operator_t

Porter-Duff 合成操作符。这些操作符控制如何将新像素与 surface 上已有内容混合：

| 值                           | 公式（简写）                           | 效果                                   |
|------------------------------|--------------------------------------|----------------------------------------|
| `CG_OPERATOR_CLEAR`          | 0                                     | 清除目标                               |
| `CG_OPERATOR_SRC`            | S                                     | 复制源                                 |
| `CG_OPERATOR_DST`            | D                                     | 保持目标不变*                           |
| `CG_OPERATOR_SRC_OVER`       | S + D*(1 - Aₛ)                        | 源在上（默认）                          |
| `CG_OPERATOR_DST_OVER`       | D + S*(1 - Aᴅ)                        | 目标在上                               |
| `CG_OPERATOR_SRC_IN`         | S * Aᴅ                                | 目标存在处显示源                        |
| `CG_OPERATOR_DST_IN`         | D * Aₛ                                | 源存在处显示目标                        |
| `CG_OPERATOR_SRC_OUT`        | S * (1 - Aᴅ)                          | 目标不存在处显示源                      |
| `CG_OPERATOR_DST_OUT`        | D * (1 - Aₛ)                          | 源不存在处显示目标                      |
| `CG_OPERATOR_SRC_ATOP`       | S*Aᴅ + D*(1 - Aₛ)                     | 源在目标之上，裁剪                      |
| `CG_OPERATOR_DST_ATOP`       | D*Aₛ + S*(1 - Aᴅ)                     | 目标在源之上，裁剪                      |
| `CG_OPERATOR_XOR`            | S*(1 - Aᴅ) + D*(1 - Aₛ)              | 非重叠区域合并                          |

*其中 S=源, D=目标, Aₛ=源透明度, Aᴅ=目标透明度*

*当 paint 为透明度=255 的纯色且操作符为 SRC_OVER 时，库内部会优化为使用 SRC 操作符。*

#### cg_paint_type_t

| 值                          | 描述         |
|----------------------------|-------------|
| `CG_PAINT_TYPE_COLOR`      | 纯色         |
| `CG_PAINT_TYPE_GRADIENT`   | 渐变         |
| `CG_PAINT_TYPE_TEXTURE`    | 纹理/图像    |

---

### 对象结构体

#### cg_surface_t

```c
struct cg_surface_t {
    int refcnt;
    int width;
    int height;
    int stride;
    int owndata;
    unsigned char * pixels;
};
```

像素画布。像素以**预乘 RGBA32** 格式存储（行优先）。`stride` 等于 `width * 4`。当 surface 通过 `cg_surface_create` 分配了自己的缓冲区时，`owndata` 为 `1`；当通过 `cg_surface_create_for_data` 包装外部数据时，`owndata` 为 `0`。

#### cg_path_t

```c
struct cg_path_t {
    int refcnt;
    int num_points;
    int num_contours;
    int num_curves;
    int sub_path;
    struct cg_point_t start_point;
    struct { union cg_path_element_t * data; int size; int capacity; } elements;
};
```

由子路径（轮廓）组成的矢量路径。每个子路径以 `MOVE_TO` 开始，可选以 `CLOSE` 结束。二次贝塞尔曲线存储为三次形式。二次转三次的转换公式为：

```
CP1 = 2/3 * Q1 + 1/3 * P0
CP2 = 2/3 * Q1 + 1/3 * P2
```

#### cg_paint_t / cg_solid_paint_t / cg_gradient_paint_t / cg_texture_paint_t

Paint 是"要绘制的内容"——可以是纯色、渐变或纹理。`cg_paint_t` 结构体作为基类，带有 `type` 区分字段。通过相应的子类型指针对具体类型进行访问。

**渐变 paint** 将颜色色标内联存储（与结构体在同一内存块中分配）。`values[6]` 存储几何信息：
- 线性：`{x1, y1, x2, y2}`
- 径向：`{cx1, cy1, r1, cx0, cy0, r0}`（先结束圆，再起始圆）

**纹理 paint** 引用一个 surface，并存储不透明度、纹理类型和仿射矩阵。

---

## 矩阵函数

矩阵将坐标从用户空间变换到 surface 空间。所有矩阵函数都在 `m` 指向的矩阵上原地操作。

### cg_matrix_init

```c
void cg_matrix_init(struct cg_matrix_t * m, float a, float b, float c, float d, float tx, float ty);
```

用显式值初始化矩阵。

### cg_matrix_init_identity

```c
void cg_matrix_init_identity(struct cg_matrix_t * m);
```

将矩阵设置为单位矩阵：`{1, 0, 0, 1, 0, 0}`。

### cg_matrix_init_translate

```c
void cg_matrix_init_translate(struct cg_matrix_t * m, float tx, float ty);
```

初始化为纯平移矩阵：`{1, 0, 0, 1, tx, ty}`。

### cg_matrix_init_scale

```c
void cg_matrix_init_scale(struct cg_matrix_t * m, float sx, float sy);
```

初始化为纯缩放矩阵：`{sx, 0, 0, sy, 0, 0}`。

### cg_matrix_init_rotate

```c
void cg_matrix_init_rotate(struct cg_matrix_t * m, float r);
```

初始化为旋转 `r` 弧度的旋转矩阵：`{cos(r), sin(r), -sin(r), cos(r), 0, 0}`。

### cg_matrix_init_shear

```c
void cg_matrix_init_shear(struct cg_matrix_t * m, float shx, float shy);
```

初始化为剪切矩阵：`{1, shy, shx, 1, 0, 0}`。

### cg_matrix_translate

```c
void cg_matrix_translate(struct cg_matrix_t * m, float tx, float ty);
```

在现有矩阵上后乘平移变换：
```
tx' = tx + a*dx + c*dy
ty' = ty + b*dx + d*dy
```

### cg_matrix_scale

```c
void cg_matrix_scale(struct cg_matrix_t * m, float sx, float sy);
```

后乘缩放变换：`a*=sx; b*=sx; c*=sy; d*=sy`。

### cg_matrix_rotate

```c
void cg_matrix_rotate(struct cg_matrix_t * m, float r);
```

后乘旋转 `r` 弧度的旋转变换。

### cg_matrix_shear

```c
void cg_matrix_shear(struct cg_matrix_t * m, float shx, float shy);
```

后乘剪切变换：`m = m * shear(shx, shy)`。

### cg_matrix_multiply

```c
void cg_matrix_multiply(struct cg_matrix_t * m, struct cg_matrix_t * m1, struct cg_matrix_t * m2);
```

设置 `m = m1 * m2`。当两个矩阵都是轴对齐（即 `b == 0` 且 `c == 0`）时，使用优化路径。

### cg_matrix_invert

```c
int cg_matrix_invert(struct cg_matrix_t * m);
```

原地求逆矩阵。成功返回非零值，矩阵奇异时返回 `0`。

特殊情况：
- 如果 `b == 0` 且 `c == 0`（轴对齐）：若 `a == 0` 或 `d == 0` 则返回 `0`。
- 否则：若 `|det| <= 1e-20` 则返回 `0`。

---

## Surface 函数

### cg_surface_create

```c
struct cg_surface_t * cg_surface_create(int width, int height);
```

分配给定大小的新 surface。像素缓冲区初始化为零（完全透明的黑色）。`owndata` 设为 `1`。

### cg_surface_create_for_data

```c
struct cg_surface_t * cg_surface_create_for_data(int width, int height, void * pixels);
```

创建包装已有像素缓冲区的 surface。`pixels` 必须至少为 `width * height * 4` 字节，格式为预乘 RGBA32。`owndata` 设为 `0`，意味着调用 `cg_surface_destroy` 将**不会**释放像素数据——调用者负责其生命周期。

### cg_surface_destroy

```c
void cg_surface_destroy(struct cg_surface_t * surface);
```

减少引用计数。当计数归零时，释放像素缓冲区（若 `owndata == 1`）和 surface 结构体。传入 `NULL` 安全。

### cg_surface_reference

```c
struct cg_surface_t * cg_surface_reference(struct cg_surface_t * surface);
```

增加引用计数并返回 surface。传入 `NULL` 安全（返回 `NULL`）。

---

## Paint 函数

Paint 对象是独立的——可以创建一次并在多个绘制操作或上下文中重用。使用完毕后调用 `cg_paint_destroy` 释放。

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

*注意：paint 中的 `values` 数组存储为 `{cx1, cy1, r1, cx0, cy0, r0}`（先结束圆，再起始圆）。*

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

---

## Path 函数

路径对象可以独立于上下文构建并重用。

### 生命周期

#### cg_path_create

```c
struct cg_path_t * cg_path_create(void);
```

创建不含任何子路径的空路径。

#### cg_path_destroy

```c
void cg_path_destroy(struct cg_path_t * path);
```

减少引用计数，归零时释放路径数据。

#### cg_path_reference

```c
struct cg_path_t * cg_path_reference(struct cg_path_t * path);
```

增加引用计数，返回路径。

### 路径操作

#### cg_path_reset

```c
void cg_path_reset(struct cg_path_t * path);
```

清空所有元素、轮廓数据和曲线计数。重置为空路径。

#### cg_path_sub_path

```c
void cg_path_sub_path(struct cg_path_t * path);
```

标记下一个绘图命令应开始一个新的子路径。如果下一个命令是 `line_to`、`cubic_to` 等，且前面没有 `move_to`，它将被提升为 `move_to`。

#### cg_path_close

```c
void cg_path_close(struct cg_path_t * path);
```

通过添加引用子路径起点的 `CLOSE` 命令来闭合当前子路径。路径为空时不做任何操作。重置 `sub_path` 标志。

#### cg_path_transform

```c
void cg_path_transform(struct cg_path_t * path, struct cg_matrix_t * m);
```

原地用矩阵 `m` 变换路径中的所有点。遍历所有 move、line、close（1 点）和 cubic（3 点）命令，将每个点通过矩阵映射。

#### cg_path_get_current_point

```c
void cg_path_get_current_point(struct cg_path_t * path, float * x, float * y);
```

返回当前位置。如果没有点或路径处于"新子路径"模式，返回 `(0, 0)`。如果只需要一个坐标，`x` 或 `y` 可以为 `NULL`。

### 构建路径

路径是增量构建的。如果路径为空，第一个绘图命令会隐式创建到 `(0, 0)` 的 `MOVE_TO`。

#### cg_path_move_to

```c
void cg_path_move_to(struct cg_path_t * path, float x, float y);
```

在 `(x, y)` 处开始一个新的子路径。增加轮廓计数。重置 `sub_path` 标志。

#### cg_path_line_to

```c
void cg_path_line_to(struct cg_path_t * path, float x, float y);
```

从当前点到 `(x, y)` 添加一条直线段。

- 如果 `sub_path` 已设置，提升为 `move_to(x, y)`。
- 如果路径为空，首先添加 `move_to(0, 0)`。

#### cg_path_quad_to

```c
void cg_path_quad_to(struct cg_path_t * path, float x1, float y1, float x2, float y2);
```

添加一条二次贝塞尔曲线。内部转换为三次：
```
CP1 = 2/3 * (x1, y1) + 1/3 * current_point
CP2 = 2/3 * (x1, y1) + 1/3 * (x2, y2)
```

如果路径为空，首先添加 `move_to(0, 0)`。

#### cg_path_cubic_to

```c
void cg_path_cubic_to(struct cg_path_t * path,
    float x1, float y1,
    float x2, float y2,
    float x3, float y3);
```

添加一条三次贝塞尔曲线，包含两个控制点 `(x1, y1)`、`(x2, y2)` 和终点 `(x3, y3)`。增加曲线计数。

如果路径为空或 `sub_path` 已设置，首先添加 `move_to(0, 0)`。

#### cg_path_arc_to

```c
void cg_path_arc_to(struct cg_path_t * path,
    float rx, float ry,
    float angle,
    int large,
    int sweep,
    float x, float y);
```

从当前点添加一条椭圆弧到 `(x, y)`。

| 参数        | 描述                                            |
|------------|-------------------------------------------------|
| `rx, ry`   | 椭圆半径（负值按绝对值处理）                      |
| `angle`    | 椭圆旋转角度，以弧度为单位                        |
| `large`    | 非零表示取大弧（弧 > 180°）                       |
| `sweep`    | 非零表示正（顺时针）角度方向                       |
| `x, y`     | 弧的终点                                        |

边界情况：
- 如果 `rx == 0` 或 `ry == 0`，或者起点等于终点，回退到 `cg_path_line_to`。
- 如果半径太小无法到达终点，会按比例放大。

### 预定义形状

#### cg_path_add_rectangle

```c
void cg_path_add_rectangle(struct cg_path_t * path, float x, float y, float w, float h);
```

添加一个闭合矩形作为新子路径：`move_to(x, y)` → `line_to(x+w, y)` → `line_to(x+w, y+h)` → `line_to(x, y+h)` → `line_to(x, y)` → `close`。

#### cg_path_add_round_rectangle

```c
void cg_path_add_round_rectangle(struct cg_path_t * path,
    float x, float y,
    float w, float h,
    float rx, float ry);
```

添加一个闭合圆角矩形。角半径被钳制到 `min(w/2, h/2)`。如果两个半径均为零，回退到 `cg_path_add_rectangle`。使用控制点在 `radius * 0.5522847498`（KAPPA）处的三次贝塞尔曲线来近似四分之一圆。

#### cg_path_add_ellipse

```c
void cg_path_add_ellipse(struct cg_path_t * path, float cx, float cy, float rx, float ry);
```

添加以 `(cx, cy)` 为中心、半径为 `(rx, ry)` 的闭合椭圆。每个椭圆使用 4 条三次贝塞尔曲线。

#### cg_path_add_circle

```c
void cg_path_add_circle(struct cg_path_t * path, float cx, float cy, float r);
```

便捷包装：调用 `cg_path_add_ellipse(path, cx, cy, r, r)`。

#### cg_path_add_arc

```c
void cg_path_add_arc(struct cg_path_t * path,
    float cx, float cy, float r,
    float a0, float a1,
    int ccw);
```

添加以 `(cx, cy)` 为中心、从角度 `a0` 到 `a1` 的圆弧。

| 参数    | 描述                        |
|--------|-----------------------------|
| `a0`   | 起始角度，以弧度为单位       |
| `a1`   | 结束角度，以弧度为单位       |
| `ccw`  | 非零表示逆时针方向           |

行为：
- 如果 `|a1 - a0| > 2π`，差值被钳制到 `2π`。
- 如果方向不匹配（例如 `a0 < a1` 但设置了 `ccw`），则加上或减去 `2π`。
- 如果路径为空或处于 `sub_path` 模式，以 `move_to` 开头到起点；否则添加 `line_to`。
- 每段 `π/2` 被细分为一条三次贝塞尔曲线。

#### cg_path_add_path

```c
void cg_path_add_path(struct cg_path_t * path,
    struct cg_path_t * source,
    struct cg_matrix_t * m);
```

将 `source` 追加到当前路径。如果 `m` 为 `NULL`，则直接复制源元素。如果 `m` 非 `NULL`，则 `source` 中的每个点在添加前都会经过 `m` 变换。

#### cg_path_extents

```c
float cg_path_extents(struct cg_path_t * path, struct cg_rect_t * extents, int tight);
```

计算路径的边界框。

| 参数        | 描述                                                       |
|------------|------------------------------------------------------------|
| `extents`  | 输出矩形，如果不需要可为 `NULL`                              |
| `tight`    | 非零：展平贝塞尔曲线以获取精确边界。零：仅使用控制点           |

**返回值**：路径的近似弧长（各段长度之和），而非面积。

---

## Context 函数

### 创建与销毁

#### cg_create

```c
struct cg_ctx_t * cg_create(struct cg_surface_t * surface);
```

创建绑定到 `surface` 的绘图上下文。surface 被引用。初始化默认状态、空路径，并将裁剪矩形设为覆盖整个 surface `{0, 0, width, height}`。

#### cg_destroy

```c
void cg_destroy(struct cg_ctx_t * ctx);
```

销毁上下文。释放所有已保存状态、空闲状态池、span 缓冲区、surface 引用和路径。传入 `NULL` 安全。

### 状态管理

绘图状态存储在链表栈上。`cg_save` 压入当前状态的副本；`cg_restore` 将其弹出。状态包括：paint、颜色、矩阵、描边样式（线宽/线帽/连接/斜接/虚线）、填充规则、操作符、不透明度和裁剪区域。

库维护一个空闲状态池以避免在 save/restore 周期中反复 `malloc`/`free`。

#### cg_save

```c
void cg_save(struct cg_ctx_t * ctx);
```

将当前状态压入栈中。所有后续对 paint、矩阵、描边样式等的修改都被隔离，直到调用 `cg_restore`。

#### cg_restore

```c
void cg_restore(struct cg_ctx_t * ctx);
```

从栈中弹出顶部状态并恢复。如果只剩下一个状态则不做任何操作（初始状态永远不会被弹出）。

### 查询函数

#### cg_get_paint

```c
struct cg_paint_t * cg_get_paint(struct cg_ctx_t * ctx, struct cg_color_t * color);
```

返回当前 paint 对象（如果使用简单颜色模式可能为 `NULL`）。如果 `color` 非 `NULL`，会从状态中存储的颜色填充值（该颜色可能与 paint 的颜色不同——状态快照颜色跟踪的是最后一次 `cg_set_source_*` 的值）。

*注意：当调用 `cg_set_source_rgba` 时，它会设置 `state->color` 为给定的 RGBA 并将 `state->paint = NULL`。因此 `cg_get_paint` 会返回 `NULL`，但 `cg_get_paint(ctx, &c)` 会用正确的颜色填充 `c`。*

#### cg_get_surface

```c
struct cg_surface_t * cg_get_surface(struct cg_ctx_t * ctx);
```

返回绑定到此上下文的 surface。

#### cg_get_path

```c
struct cg_path_t * cg_get_path(struct cg_ctx_t * ctx);
```

返回正在构建的当前路径。

#### cg_get_matrix

```c
struct cg_matrix_t * cg_get_matrix(struct cg_ctx_t * ctx);
```

返回指向当前变换矩阵的指针（可修改——改动会影响上下文）。

#### cg_get_operator

```c
enum cg_operator_t cg_get_operator(struct cg_ctx_t * ctx);
```

返回当前合成操作符。

#### cg_get_opacity

```c
float cg_get_opacity(struct cg_ctx_t * ctx);
```

返回当前全局不透明度。

#### cg_get_current_point

```c
void cg_get_current_point(struct cg_ctx_t * ctx, float * x, float * y);
```

返回当前路径位置。

### 空间查询

这些函数对当前路径进行光栅化以确定空间关系。如果在循环中反复调用，可能开销较大。

#### cg_fill_extents

```c
void cg_fill_extents(struct cg_ctx_t * ctx, struct cg_rect_t * extents);
```

使用当前填充规则光栅化当前路径，并将边界框写入 `extents`。

#### cg_stroke_extents

```c
void cg_stroke_extents(struct cg_ctx_t * ctx, struct cg_rect_t * extents);
```

使用当前描边样式（线宽、线帽、连接、虚线）光栅化当前路径，并将边界框写入 `extents`。

#### cg_clip_extents

```c
void cg_clip_extents(struct cg_ctx_t * ctx, struct cg_rect_t * extents);
```

返回当前裁剪区域的边界框。如果裁剪已激活，返回裁剪 span 的边界。否则返回初始裁剪矩形。

#### cg_in_fill

```c
int cg_in_fill(struct cg_ctx_t * ctx, float x, float y);
```

检测点 `(x, y)` 是否在当前路径的填充区域内。

#### cg_in_stroke

```c
int cg_in_stroke(struct cg_ctx_t * ctx, float x, float y);
```

检测点 `(x, y)` 是否在当前路径的描边区域内。

#### cg_in_clip

```c
int cg_in_clip(struct cg_ctx_t * ctx, float x, float y);
```

检测点 `(x, y)` 是否在当前裁剪区域内。

### Paint 与样式

#### cg_set_paint

```c
void cg_set_paint(struct cg_ctx_t * ctx, struct cg_paint_t * paint);
```

设置当前 paint。paint 被引用。传入 `NULL` 恢复使用状态中存储的颜色。

#### cg_set_source_rgb

```c
void cg_set_source_rgb(struct cg_ctx_t * ctx, float r, float g, float b);
```

将 paint 设置为纯 RGB 颜色（不透明度 = 1.0）。内部将颜色存储在状态中，并将 paint 设为 `NULL`。

#### cg_set_source_rgba

```c
void cg_set_source_rgba(struct cg_ctx_t * ctx, float r, float g, float b, float a);
```

将 paint 设置为纯 RGBA 颜色。

#### cg_set_source_color

```c
void cg_set_source_color(struct cg_ctx_t * ctx, struct cg_color_t * color);
```

从 `cg_color_t` 结构体设置纯色 paint。

#### cg_set_source_surface

```c
void cg_set_source_surface(struct cg_ctx_t * ctx, struct cg_surface_t * surface, float x, float y);
```

从 `surface` 创建一个普通（非平铺）纹理 paint，带有 `(x, y)` 的平移偏移。surface 被引用。内部等价于：

```c
cg_set_texture(ctx, surface, CG_TEXTURE_TYPE_PLAIN, 1.0, &translate_matrix);
```

#### cg_set_linear_gradient

```c
void cg_set_linear_gradient(struct cg_ctx_t * ctx,
    float x1, float y1, float x2, float y2,
    enum cg_spread_method_t spread,
    struct cg_gradient_stop_t * stops, int nstops,
    struct cg_matrix_t * m);
```

创建并设置线性渐变 paint。参数详情参见 `cg_paint_create_linear_gradient`。

#### cg_set_radial_gradient

```c
void cg_set_radial_gradient(struct cg_ctx_t * ctx,
    float cx0, float cy0, float r0,
    float cx1, float cy1, float r1,
    enum cg_spread_method_t spread,
    struct cg_gradient_stop_t * stops, int nstops,
    struct cg_matrix_t * m);
```

创建并设置径向渐变 paint。参数详情参见 `cg_paint_create_radial_gradient`。

#### cg_set_texture

```c
void cg_set_texture(struct cg_ctx_t * ctx,
    struct cg_surface_t * surface,
    enum cg_texture_type_t type,
    float opacity,
    struct cg_matrix_t * m);
```

创建并设置纹理 paint。参数详情参见 `cg_paint_create_texture`。

#### cg_set_operator

```c
void cg_set_operator(struct cg_ctx_t * ctx, enum cg_operator_t op);
```

设置混合到 surface 时使用的合成操作符。

#### cg_set_opacity

```c
void cg_set_opacity(struct cg_ctx_t * ctx, float opacity);
```

设置全局不透明度。钳制到 `[0.0, 1.0]`。此值与 paint 的不透明度和每个 span 的覆盖率相乘。

#### cg_set_fill_rule

```c
void cg_set_fill_rule(struct cg_ctx_t * ctx, enum cg_fill_rule_t winding);
```

设置填充规则。同时适用于填充和裁剪。

#### cg_set_line_width

```c
void cg_set_line_width(struct cg_ctx_t * ctx, float width);
```

设置描边线宽。线宽受当前变换矩阵影响（缩放 CTM 会改变表观描边宽度）。

#### cg_set_line_cap

```c
void cg_set_line_cap(struct cg_ctx_t * ctx, enum cg_line_cap_t cap);
```

设置开放子路径端点的绘制方式。

#### cg_set_line_join

```c
void cg_set_line_join(struct cg_ctx_t * ctx, enum cg_line_join_t join);
```

设置连接线段之间拐角的绘制方式。

#### cg_set_miter_limit

```c
void cg_set_miter_limit(struct cg_ctx_t * ctx, float limit);
```

设置斜接限制。当使用 `CG_LINE_JOIN_MITER` 连接线段时，如果斜接延伸将超过 `limit * line_width / 2`，连接将被转换为平角。

#### cg_set_dash

```c
void cg_set_dash(struct cg_ctx_t * ctx, float * dashes, int ndashes, float offset);
```

设置虚线模式。`dashes` 是一个交替的实线/空白长度数组（用户空间单位）。如果数组有奇数个元素，实际上会被加倍。`offset` 指定从模式中的哪个位置开始。数组和偏移量可分别通过 `cg_set_dash_array` 和 `cg_set_dash_offset` 独立设置。

要禁用虚线，将 `ndashes` 设为 `0`。

#### cg_set_dash_array

```c
void cg_set_dash_array(struct cg_ctx_t * ctx, float * dashes, int ndashes);
```

仅设置虚线数组，保留当前偏移量。

#### cg_set_dash_offset

```c
void cg_set_dash_offset(struct cg_ctx_t * ctx, float offset);
```

仅设置虚线偏移量，保留当前数组。

### 变换

所有变换函数将给定的变换后乘到当前变换矩阵（CTM）上。这意味着变换按调用顺序应用——最后调用的变换最先生效。

#### cg_translate

```c
void cg_translate(struct cg_ctx_t * ctx, float tx, float ty);
```

将 CTM 平移 `(tx, ty)`。

#### cg_scale

```c
void cg_scale(struct cg_ctx_t * ctx, float sx, float sy);
```

将 CTM 缩放 `(sx, sy)`。

#### cg_shear

```c
void cg_shear(struct cg_ctx_t * ctx, float shx, float shy);
```

剪切 CTM。

#### cg_rotate

```c
void cg_rotate(struct cg_ctx_t * ctx, float angle);
```

将 CTM 旋转 `angle` 弧度，顺时针方向。

#### cg_transform

```c
void cg_transform(struct cg_ctx_t * ctx, struct cg_matrix_t * m);
```

将 CTM 后乘 `m`：`CTM = m * CTM`。

#### cg_set_matrix

```c
void cg_set_matrix(struct cg_ctx_t * ctx, struct cg_matrix_t * m);
```

用 `m` 替换 CTM。

#### cg_identity_matrix

```c
void cg_identity_matrix(struct cg_ctx_t * ctx);
```

将 CTM 重置为单位矩阵。

### 路径构建

这些函数构建当前路径。坐标在用户空间中，在渲染时会被 CTM 变换。

#### 基本操作

##### cg_move_to

```c
void cg_move_to(struct cg_ctx_t * ctx, float x, float y);
```

在 `(x, y)` 处开始新的子路径。

##### cg_line_to

```c
void cg_line_to(struct cg_ctx_t * ctx, float x, float y);
```

添加一条到 `(x, y)` 的直线段。

##### cg_quad_to

```c
void cg_quad_to(struct cg_ctx_t * ctx, float x1, float y1, float x2, float y2);
```

添加二次贝塞尔曲线，控制点为 `(x1, y1)`，终点为 `(x2, y2)`。内部转换为三次。

##### cg_cubic_to

```c
void cg_cubic_to(struct cg_ctx_t * ctx,
    float x1, float y1,
    float x2, float y2,
    float x3, float y3);
```

添加三次贝塞尔曲线。

##### cg_arc_to

```c
void cg_arc_to(struct cg_ctx_t * ctx,
    float rx, float ry, float angle,
    int large, int sweep,
    float x, float y);
```

添加椭圆弧。详情参见 `cg_path_arc_to`。

#### 相对坐标操作

这些函数通过将增量加到当前点来计算目标坐标：

```c
void cg_rel_move_to(struct cg_ctx_t * ctx, float dx, float dy);
void cg_rel_line_to(struct cg_ctx_t * ctx, float dx, float dy);
void cg_rel_quad_to(struct cg_ctx_t * ctx, float dx1, float dy1, float dx2, float dy2);
void cg_rel_cubic_to(struct cg_ctx_t * ctx, float dx1, float dy1, float dx2, float dy2, float dx3, float dy3);
void cg_rel_arc_to(struct cg_ctx_t * ctx, float rx, float ry, float angle, int large, int sweep, float dx, float dy);
```

例如，`cg_rel_line_to(ctx, 10, 0)` 从当前点向右画 10 单位的直线。

#### 预定义形状

以下每个函数都添加一个完整的闭合子路径：

```c
void cg_rectangle(struct cg_ctx_t * ctx, float x, float y, float w, float h);
void cg_round_rectangle(struct cg_ctx_t * ctx, float x, float y, float w, float h, float rx, float ry);
void cg_ellipse(struct cg_ctx_t * ctx, float cx, float cy, float rx, float ry);
void cg_circle(struct cg_ctx_t * ctx, float cx, float cy, float r);
void cg_arc(struct cg_ctx_t * ctx, float cx, float cy, float r, float a0, float a1);
void cg_arc_negative(struct cg_ctx_t * ctx, float cx, float cy, float r, float a0, float a1);
```

`cg_arc` 沿正（顺时针）方向绘制；`cg_arc_negative` 沿逆时针方向绘制。

#### 路径管理

##### cg_new_path

```c
void cg_new_path(struct cg_ctx_t * ctx);
```

清除所有线段，将路径重置为空。

##### cg_new_sub_path

```c
void cg_new_sub_path(struct cg_ctx_t * ctx);
```

开始新的子路径。下一个绘图命令将从给定点开始。

##### cg_close_path

```c
void cg_close_path(struct cg_ctx_t * ctx);
```

用一条直线将当前子路径闭合回到其起点。

##### cg_add_path

```c
void cg_add_path(struct cg_ctx_t * ctx, struct cg_path_t * path);
```

将外部路径追加到当前路径。路径元素直接复制（无变换）。

### 渲染

#### cg_clip

```c
void cg_clip(struct cg_ctx_t * ctx);
```

使用当前填充规则光栅化当前路径，将其设为裁剪区域，替换任何之前的裁剪区域。然后清除路径。后续绘制仅在裁剪区域内可见。

*内部机制：裁剪区域以 span 缓冲区的形式存储。首次裁剪调用时，路径被光栅化并存储。后续的裁剪调用与现有裁剪 span 取交集。*

#### cg_clip_preserve

```c
void cg_clip_preserve(struct cg_ctx_t * ctx);
```

与 `cg_clip` 相同，但不清除路径（适用于裁剪后再填充+描边）。

#### cg_fill

```c
void cg_fill(struct cg_ctx_t * ctx);
```

使用当前填充规则光栅化当前路径，然后将 paint 混合到填充区域内的 surface 上（与活动裁剪区域取交集）。之后清除路径。

渲染管线：路径 → FT outline → 光栅化为 span → 与裁剪 span 取交集（如有）→ 用活动操作符/不透明度混合。

#### cg_fill_preserve

```c
void cg_fill_preserve(struct cg_ctx_t * ctx);
```

与 `cg_fill` 相同，但保留路径以便后续操作（例如先填充再描边）。

#### cg_stroke

```c
void cg_stroke(struct cg_ctx_t * ctx);
```

使用当前描边样式（线宽、线帽、连接、斜接限制、虚线模式）对当前路径进行描边。如果设置了虚线，路径首先被虚线化，然后使用底层 FreeType stroker 进行描边。清除路径。

#### cg_stroke_preserve

```c
void cg_stroke_preserve(struct cg_ctx_t * ctx);
```

与 `cg_stroke` 相同，但保留路径。

#### cg_paint

```c
void cg_paint(struct cg_ctx_t * ctx);
```

用当前 paint 填充整个裁剪区域（如果未设置裁剪，则为整个 surface）。不使用路径。通常用于用纹理或渐变覆盖整个区域。

---

## 完整示例

### 基础：红色矩形配蓝色边框

```c
#include <cg.h>

struct cg_surface_t * surface = cg_surface_create(400, 300);
struct cg_ctx_t * ctx = cg_create(surface);

cg_set_source_rgba(ctx, 1.0, 0.0, 0.0, 1.0);
cg_rectangle(ctx, 50, 50, 100, 80);
cg_fill(ctx);

cg_set_source_rgba(ctx, 0.0, 0.0, 1.0, 1.0);
cg_set_line_width(ctx, 3.0);
cg_rectangle(ctx, 50, 50, 100, 80);
cg_stroke(ctx);

cg_destroy(ctx);
cg_surface_destroy(surface);
```

### 使用裁剪

```c
struct cg_surface_t * surface = cg_surface_create(256, 256);
struct cg_ctx_t * ctx = cg_create(surface);

cg_arc(ctx, 128, 128, 76.8, 0, 2 * M_PI);
cg_clip(ctx);

cg_rectangle(ctx, 0, 0, 256, 256);
cg_fill(ctx);

cg_set_source_rgb(ctx, 0, 1, 0);
cg_move_to(ctx, 0, 0);
cg_line_to(ctx, 256, 256);
cg_set_line_width(ctx, 10.0);
cg_stroke(ctx);

cg_destroy(ctx);
cg_surface_destroy(surface);
```

### 线性渐变

```c
struct cg_surface_t * surface = cg_surface_create(256, 256);
struct cg_ctx_t * ctx = cg_create(surface);

struct cg_gradient_stop_t stops[] = {
    { 0.0, { 1, 1, 1, 1 } },
    { 1.0, { 0, 0, 0, 1 } },
};

cg_rectangle(ctx, 0, 0, 256, 256);
cg_set_linear_gradient(ctx, 0, 0, 0, 256,
    CG_SPREAD_METHOD_PAD, stops, 2, NULL);
cg_fill(ctx);

cg_destroy(ctx);
cg_surface_destroy(surface);
```

### 使用 Save/Restore 的变换

```c
struct cg_surface_t * surface = cg_surface_create(256, 256);
struct cg_ctx_t * ctx = cg_create(surface);

cg_set_source_rgb(ctx, 1, 0, 0);

cg_save(ctx);
cg_translate(ctx, 64, 64);
cg_rotate(ctx, M_PI / 4);
cg_rectangle(ctx, -30, -20, 60, 40);
cg_fill(ctx);
cg_restore(ctx);

cg_save(ctx);
cg_translate(ctx, 192, 192);
cg_scale(ctx, 2.0, 1.0);
cg_rectangle(ctx, -30, -20, 60, 40);
cg_fill(ctx);
cg_restore(ctx);

cg_destroy(ctx);
cg_surface_destroy(surface);
```

### 描边样式：线帽、连接和虚线

```c
struct cg_surface_t * surface = cg_surface_create(256, 256);
struct cg_ctx_t * ctx = cg_create(surface);

float dashes[] = { 50, 10, 10, 10 };
cg_set_dash(ctx, dashes, 4, -50);
cg_set_line_width(ctx, 10);
cg_set_line_cap(ctx, CG_LINE_CAP_ROUND);
cg_set_line_join(ctx, CG_LINE_JOIN_ROUND);

cg_move_to(ctx, 128, 25.6);
cg_line_to(ctx, 230.4, 230.4);
cg_rel_line_to(ctx, -102.4, 0);
cg_cubic_to(ctx, 51.2, 230.4, 51.2, 128, 128, 128);
cg_stroke(ctx);

cg_destroy(ctx);
cg_surface_destroy(surface);
```

### 合成操作符

```c
cg_set_operator(ctx, CG_OPERATOR_SRC_OVER);
cg_set_source_rgba(ctx, 1.0, 0.0, 0.0, 0.7);
cg_arc(ctx, cx, cy, r, 0, 2 * M_PI);
cg_fill(ctx);
```

### 平铺纹理与变换

```c
struct cg_surface_t * img = cg_surface_create_for_data(128, 128, pixels);
struct cg_surface_t * surface = cg_surface_create(256, 256);
struct cg_ctx_t * ctx = cg_create(surface);

cg_translate(ctx, 128, 128);
cg_rotate(ctx, -45 * M_PI / 180);
cg_scale(ctx, 0.8, 0.8);
cg_translate(ctx, -64, -64);
cg_set_texture(ctx, img, CG_TEXTURE_TYPE_TILED, 1.0, NULL);
cg_paint(ctx);

cg_destroy(ctx);
cg_surface_destroy(surface);
cg_surface_destroy(img);
```
