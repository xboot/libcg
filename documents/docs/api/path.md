# Path 函数

路径对象可以独立于上下文构建并重用。路径由子路径（轮廓）组成，每个子路径以 `MOVE_TO` 开始，可选以 `CLOSE` 结束。

## 生命周期

### cg_path_create

```c
struct cg_path_t * cg_path_create(void);
```

创建不含任何子路径的空路径。

### cg_path_destroy

```c
void cg_path_destroy(struct cg_path_t * path);
```

减少引用计数，归零时释放路径数据。

### cg_path_reference

```c
struct cg_path_t * cg_path_reference(struct cg_path_t * path);
```

增加引用计数，返回路径。

## 路径操作

### cg_path_reset

```c
void cg_path_reset(struct cg_path_t * path);
```

清空所有元素、轮廓数据和曲线计数。重置为空路径。

### cg_path_sub_path

```c
void cg_path_sub_path(struct cg_path_t * path);
```

标记下一个绘图命令应开始一个新的子路径。如果下一个命令是 `line_to`、`cubic_to` 等，且前面没有 `move_to`，它将被提升为 `move_to`。

### cg_path_close

```c
void cg_path_close(struct cg_path_t * path);
```

通过添加引用子路径起点的 `CLOSE` 命令来闭合当前子路径。路径为空时不做任何操作。重置 `sub_path` 标志。

### cg_path_transform

```c
void cg_path_transform(struct cg_path_t * path, struct cg_matrix_t * m);
```

原地用矩阵 `m` 变换路径中的所有点。遍历所有 move、line、close（1 点）和 cubic（3 点）命令，将每个点通过矩阵映射。

### cg_path_get_current_point

```c
void cg_path_get_current_point(struct cg_path_t * path, float * x, float * y);
```

返回当前位置。如果没有点或路径处于"新子路径"模式，返回 `(0, 0)`。如果只需要一个坐标，`x` 或 `y` 可以为 `NULL`。

## 构建路径

路径是增量构建的。如果路径为空，第一个绘图命令会隐式创建到 `(0, 0)` 的 `MOVE_TO`。

### cg_path_move_to

```c
void cg_path_move_to(struct cg_path_t * path, float x, float y);
```

在 `(x, y)` 处开始一个新的子路径。增加轮廓计数。重置 `sub_path` 标志。

### cg_path_line_to

```c
void cg_path_line_to(struct cg_path_t * path, float x, float y);
```

从当前点到 `(x, y)` 添加一条直线段。

- 如果 `sub_path` 已设置，提升为 `move_to(x, y)`。
- 如果路径为空，首先添加 `move_to(0, 0)`。

### cg_path_quad_to

```c
void cg_path_quad_to(struct cg_path_t * path, float x1, float y1, float x2, float y2);
```

添加一条二次贝塞尔曲线。内部转换为三次：
```
CP1 = 2/3 * (x1, y1) + 1/3 * current_point
CP2 = 2/3 * (x1, y1) + 1/3 * (x2, y2)
```

如果路径为空，首先添加 `move_to(0, 0)`。

### cg_path_cubic_to

```c
void cg_path_cubic_to(struct cg_path_t * path,
    float x1, float y1,
    float x2, float y2,
    float x3, float y3);
```

添加一条三次贝塞尔曲线，包含两个控制点 `(x1, y1)`、`(x2, y2)` 和终点 `(x3, y3)`。增加曲线计数。

如果路径为空或 `sub_path` 已设置，首先添加 `move_to(0, 0)`。

### cg_path_arc_to

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

## 预定义形状

### cg_path_add_rectangle

```c
void cg_path_add_rectangle(struct cg_path_t * path, float x, float y, float w, float h);
```

添加一个闭合矩形作为新子路径：`move_to(x, y)` → `line_to(x+w, y)` → `line_to(x+w, y+h)` → `line_to(x, y+h)` → `line_to(x, y)` → `close`。

### cg_path_add_round_rectangle

```c
void cg_path_add_round_rectangle(struct cg_path_t * path,
    float x, float y,
    float w, float h,
    float rx, float ry);
```

添加一个闭合圆角矩形。角半径被钳制到 `min(w/2, h/2)`。如果两个半径均为零，回退到 `cg_path_add_rectangle`。使用控制点在 `radius * 0.5522847498`（KAPPA）处的三次贝塞尔曲线来近似四分之一圆。

### cg_path_add_ellipse

```c
void cg_path_add_ellipse(struct cg_path_t * path, float cx, float cy, float rx, float ry);
```

添加以 `(cx, cy)` 为中心、半径为 `(rx, ry)` 的闭合椭圆。每个椭圆使用 4 条三次贝塞尔曲线。

### cg_path_add_circle

```c
void cg_path_add_circle(struct cg_path_t * path, float cx, float cy, float r);
```

便捷包装：调用 `cg_path_add_ellipse(path, cx, cy, r, r)`。

### cg_path_add_arc

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

### cg_path_add_path

```c
void cg_path_add_path(struct cg_path_t * path,
    struct cg_path_t * source,
    struct cg_matrix_t * m);
```

将 `source` 追加到当前路径。如果 `m` 为 `NULL`，则直接复制源元素。如果 `m` 非 `NULL`，则 `source` 中的每个点在添加前都会经过 `m` 变换。

### cg_path_extents

```c
float cg_path_extents(struct cg_path_t * path, struct cg_rect_t * extents, int tight);
```

计算路径的边界框。

| 参数        | 描述                                                       |
|------------|------------------------------------------------------------|
| `extents`  | 输出矩形，如果不需要可为 `NULL`                              |
| `tight`    | 非零：展平贝塞尔曲线以获取精确边界。零：仅使用控制点           |

**返回值**：路径的近似弧长（各段长度之和），而非面积。
