# 数据类型

本文档列出 libcg 中所有基础结构体、枚举类型和对象结构体的定义。

## 基础结构体

### cg_point_t

```c
struct cg_point_t {
    float x;
    float y;
};
```

二维空间中的一个点。

### cg_rect_t

```c
struct cg_rect_t {
    float x;
    float y;
    float w;
    float h;
};
```

一个轴对齐矩形，锚定在左上角 `(x, y)`，宽度为 `w`，高度为 `h`。

### cg_matrix_t

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

### cg_color_t

```c
struct cg_color_t {
    float r;
    float g;
    float b;
    float a;
};
```

RGBA 颜色。构建 paint 时所有分量会被钳制到 `[0.0, 1.0]`。

### cg_gradient_stop_t

```c
struct cg_gradient_stop_t {
    float offset;
    struct cg_color_t color;
};
```

定义渐变中的一个位置-颜色对。`offset` 必须在 `[0.0, 1.0]` 范围内。偏移量会在内部被钳制和排序——如果某个色标的偏移量小于前一个色标，它会被提升到前一个色标的值。

## 枚举类型

### cg_path_command_t

内部用于描述路径元素：

| 值                          | 点数 | 描述                      |
|-----------------------------|------|---------------------------|
| `CG_PATH_COMMAND_MOVE_TO`   | 1    | 移动到新点，开始一个子路径   |
| `CG_PATH_COMMAND_LINE_TO`   | 1    | 直线到目标点               |
| `CG_PATH_COMMAND_CUBIC_TO`  | 3    | 三次贝塞尔曲线（控制点1、控制点2、终点） |
| `CG_PATH_COMMAND_CLOSE`     | 1    | 闭合当前子路径             |

:::note
二次贝塞尔曲线在添加到路径时会内部转换为三次。
:::

### cg_spread_method_t

控制渐变颜色在 `[0, 1]` 范围外如何采样：

| 值                          | 描述                      |
|-----------------------------|---------------------------|
| `CG_SPREAD_METHOD_PAD`      | 钳制到最近的边缘颜色        |
| `CG_SPREAD_METHOD_REFLECT`  | 在每个整数边界处镜像渐变     |
| `CG_SPREAD_METHOD_REPEAT`   | 重复渐变（环绕）            |

### cg_gradient_type_t

| 值                          | 描述     |
|-----------------------------|---------|
| `CG_GRADIENT_TYPE_LINEAR`   | 线性渐变 |
| `CG_GRADIENT_TYPE_RADIAL`   | 径向渐变 |

### cg_texture_type_t

| 值                         | 描述                                          |
|---------------------------|-----------------------------------------------|
| `CG_TEXTURE_TYPE_PLAIN`   | 单次绘制纹理（区域外为透明）                      |
| `CG_TEXTURE_TYPE_TILED`   | 纹理重复以填充整个绘制区域                        |

:::note
经过变换的非平铺纹理，以及带有旋转/剪切的平铺纹理，使用双线性过滤。其他组合使用最近邻采样。
:::

### cg_line_cap_t

| 值                       | 描述                                |
|--------------------------|-------------------------------------|
| `CG_LINE_CAP_BUTT`       | 平头端点，恰好在端点处结束（默认）      |
| `CG_LINE_CAP_ROUND`      | 半圆形端点，超出端点延伸              |
| `CG_LINE_CAP_SQUARE`     | 方形端点，超出端点延伸半个线宽         |

### cg_line_join_t

| 值                        | 描述                                  |
|---------------------------|---------------------------------------|
| `CG_LINE_JOIN_MITER`      | 尖角，受斜接限制裁剪（默认）             |
| `CG_LINE_JOIN_ROUND`      | 圆角                                  |
| `CG_LINE_JOIN_BEVEL`      | 平角（斜角）                           |

### cg_fill_rule_t

确定自相交路径中哪些区域属于"内部"：

| 值                          | 描述                    |
|-----------------------------|------------------------|
| `CG_FILL_RULE_NON_ZERO`     | 非零环绕规则（默认）      |
| `CG_FILL_RULE_EVEN_ODD`     | 奇偶规则                |

### cg_operator_t

Porter-Duff 合成操作符。这些操作符控制如何将新像素与 surface 上已有内容混合。详见[合成操作符参考](../reference/operators)。

| 值                           | 效果                                   |
|------------------------------|----------------------------------------|
| `CG_OPERATOR_CLEAR`          | 清除目标                               |
| `CG_OPERATOR_SRC`            | 复制源                                 |
| `CG_OPERATOR_DST`            | 保持目标不变                           |
| `CG_OPERATOR_SRC_OVER`       | 源在上（默认）                          |
| `CG_OPERATOR_DST_OVER`       | 目标在上                               |
| `CG_OPERATOR_SRC_IN`         | 目标存在处显示源                        |
| `CG_OPERATOR_DST_IN`         | 源存在处显示目标                        |
| `CG_OPERATOR_SRC_OUT`        | 目标不存在处显示源                      |
| `CG_OPERATOR_DST_OUT`        | 源不存在处显示目标                      |
| `CG_OPERATOR_SRC_ATOP`       | 源在目标之上，裁剪                      |
| `CG_OPERATOR_DST_ATOP`       | 目标在源之上，裁剪                      |
| `CG_OPERATOR_XOR`            | 非重叠区域合并                          |

:::note
当 paint 为透明度=255 的纯色且操作符为 SRC_OVER 时，库内部会优化为使用 SRC 操作符。
:::

### cg_paint_type_t

| 值                          | 描述         |
|----------------------------|-------------|
| `CG_PAINT_TYPE_COLOR`      | 纯色         |
| `CG_PAINT_TYPE_GRADIENT`   | 渐变         |
| `CG_PAINT_TYPE_TEXTURE`    | 纹理/图像    |

## 对象结构体

### cg_surface_t

```c
struct cg_surface_t {
    int refcnt;
    int width;
    int height;
    int stride;
    int owns;
    unsigned char * pixels;
};
```

像素画布。像素以**预乘 RGBA32** 格式存储（行优先）。`stride` 等于 `width * 4`。当 surface 通过 `cg_surface_create` 分配了自己的缓冲区时，`owns` 为 `1`；当通过 `cg_surface_create_for_data` 包装外部数据时，`owns` 为 `0`。

### cg_path_t

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

### cg_paint_t / cg_solid_paint_t / cg_gradient_paint_t / cg_texture_paint_t

Paint 是"要绘制的内容"——可以是纯色、渐变或纹理。`cg_paint_t` 结构体作为基类，带有 `type` 区分字段。通过相应的子类型指针对具体类型进行访问。

**渐变 paint** 将颜色色标内联存储（与结构体在同一内存块中分配）。`values[6]` 存储几何信息：
- 线性：`{x1, y1, x2, y2}`
- 径向：`{cx0, cy0, r0, cx1, cy1, r1}`（先起始圆，再结束圆）

**纹理 paint** 引用一个 surface，并存储不透明度、纹理类型和仿射矩阵。
