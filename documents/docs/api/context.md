# Context 函数

绘图上下文 `cg_ctx_t` 是 libcg 的核心对象，所有绘图操作都通过上下文完成。它持有目标 surface 引用、当前路径和状态栈。

## 创建与销毁

### cg_create

```c
struct cg_ctx_t * cg_create(struct cg_surface_t * surface);
```

创建绑定到 `surface` 的绘图上下文。surface 被引用。初始化默认状态、空路径，并将裁剪矩形设为覆盖整个 surface `{0, 0, width, height}`。

### cg_destroy

```c
void cg_destroy(struct cg_ctx_t * ctx);
```

销毁上下文。释放所有已保存状态、空闲状态池、span 缓冲区、surface 引用和路径。传入 `NULL` 安全。

## 状态管理

绘图状态存储在链表栈上。`cg_save` 压入当前状态的副本；`cg_restore` 将其弹出。状态包括：paint、颜色、矩阵、描边样式（线宽/线帽/连接/斜接/虚线）、填充规则、操作符、不透明度和裁剪区域。

库维护一个空闲状态池以避免在 save/restore 周期中反复 `malloc`/`free`。

### cg_save

```c
void cg_save(struct cg_ctx_t * ctx);
```

将当前状态压入栈中。所有后续对 paint、矩阵、描边样式等的修改都被隔离，直到调用 `cg_restore`。

### cg_restore

```c
void cg_restore(struct cg_ctx_t * ctx);
```

从栈中弹出顶部状态并恢复。如果只剩下一个状态则不做任何操作（初始状态永远不会被弹出）。

## 查询函数

### cg_get_source

```c
struct cg_paint_t * cg_get_source(struct cg_ctx_t * ctx, struct cg_color_t * color);
```

返回当前 paint 对象（如果使用简单颜色模式可能为 `NULL`）。如果 `color` 非 `NULL`，会从状态中存储的颜色填充值。

:::note
当调用 `cg_set_source_rgba` 时，它会设置 `state->color` 为给定的 RGBA 并将 `state->paint = NULL`。因此 `cg_get_source` 会返回 `NULL`，但 `cg_get_source(ctx, &c)` 会用正确的颜色填充 `c`。
:::

### cg_get_surface

```c
struct cg_surface_t * cg_get_surface(struct cg_ctx_t * ctx);
```

返回绑定到此上下文的 surface。

### cg_get_path

```c
struct cg_path_t * cg_get_path(struct cg_ctx_t * ctx);
```

返回正在构建的当前路径。

### cg_get_matrix

```c
struct cg_matrix_t * cg_get_matrix(struct cg_ctx_t * ctx);
```

返回指向当前变换矩阵的指针（可修改——改动会影响上下文）。

### cg_get_operator

```c
enum cg_operator_t cg_get_operator(struct cg_ctx_t * ctx);
```

返回当前合成操作符。

### cg_get_opacity

```c
float cg_get_opacity(struct cg_ctx_t * ctx);
```

返回当前全局不透明度。

### cg_get_current_point

```c
void cg_get_current_point(struct cg_ctx_t * ctx, float * x, float * y);
```

返回当前路径位置。

### cg_has_current_point

```c
int cg_has_current_point(struct cg_ctx_t * ctx);
```

检查当前路径是否有当前点。如果有当前点则返回 1，否则返回 0。

## 空间查询

这些函数对当前路径进行光栅化以确定空间关系。如果在循环中反复调用，可能开销较大。

### cg_fill_extents

```c
void cg_fill_extents(struct cg_ctx_t * ctx, struct cg_rect_t * extents);
```

使用当前填充规则光栅化当前路径，并将边界框写入 `extents`。

### cg_stroke_extents

```c
void cg_stroke_extents(struct cg_ctx_t * ctx, struct cg_rect_t * extents);
```

使用当前描边样式光栅化当前路径，并将边界框写入 `extents`。

### cg_clip_extents

```c
void cg_clip_extents(struct cg_ctx_t * ctx, struct cg_rect_t * extents);
```

返回当前裁剪区域的边界框。

### cg_in_fill

```c
int cg_in_fill(struct cg_ctx_t * ctx, float x, float y);
```

检测点 `(x, y)` 是否在当前路径的填充区域内。

### cg_in_stroke

```c
int cg_in_stroke(struct cg_ctx_t * ctx, float x, float y);
```

检测点 `(x, y)` 是否在当前路径的描边区域内。

### cg_in_clip

```c
int cg_in_clip(struct cg_ctx_t * ctx, float x, float y);
```

检测点 `(x, y)` 是否在当前裁剪区域内。

## Paint 与样式

### cg_set_source

```c
void cg_set_source(struct cg_ctx_t * ctx, struct cg_paint_t * paint);
```

设置当前 paint。paint 被引用。传入 `NULL` 恢复使用状态中存储的颜色。

### cg_set_source_rgb

```c
void cg_set_source_rgb(struct cg_ctx_t * ctx, float r, float g, float b);
```

将 paint 设置为纯 RGB 颜色（不透明度 = 1.0）。

### cg_set_source_rgba

```c
void cg_set_source_rgba(struct cg_ctx_t * ctx, float r, float g, float b, float a);
```

将 paint 设置为纯 RGBA 颜色。

### cg_set_source_surface

```c
void cg_set_source_surface(struct cg_ctx_t * ctx, struct cg_surface_t * surface, float x, float y);
```

从 `surface` 创建一个普通（非平铺）纹理 paint，带有 `(x, y)` 的平移偏移。内部等价于：

```c
cg_set_texture(ctx, surface, CG_TEXTURE_TYPE_PLAIN, 1.0, &translate_matrix);
```

### cg_set_linear_gradient

```c
void cg_set_linear_gradient(struct cg_ctx_t * ctx,
    float x1, float y1, float x2, float y2,
    enum cg_spread_method_t spread,
    struct cg_gradient_stop_t * stops, int nstops,
    struct cg_matrix_t * m);
```

创建并设置线性渐变 paint。参数详情参见 [cg_paint_create_linear_gradient](./paint#cg_paint_create_linear_gradient)。

### cg_set_radial_gradient

```c
void cg_set_radial_gradient(struct cg_ctx_t * ctx,
    float cx0, float cy0, float r0,
    float cx1, float cy1, float r1,
    enum cg_spread_method_t spread,
    struct cg_gradient_stop_t * stops, int nstops,
    struct cg_matrix_t * m);
```

创建并设置径向渐变 paint。参数详情参见 [cg_paint_create_radial_gradient](./paint#cg_paint_create_radial_gradient)。

### cg_set_texture

```c
void cg_set_texture(struct cg_ctx_t * ctx,
    struct cg_surface_t * surface,
    enum cg_texture_type_t type,
    float opacity,
    struct cg_matrix_t * m);
```

创建并设置纹理 paint。参数详情参见 [cg_paint_create_texture](./paint#cg_paint_create_texture)。

### cg_set_operator

```c
void cg_set_operator(struct cg_ctx_t * ctx, enum cg_operator_t op);
```

设置混合到 surface 时使用的合成操作符。

### cg_set_opacity

```c
void cg_set_opacity(struct cg_ctx_t * ctx, float opacity);
```

设置全局不透明度。钳制到 `[0.0, 1.0]`。此值与 paint 的不透明度和每个 span 的覆盖率相乘。

### cg_set_fill_rule

```c
void cg_set_fill_rule(struct cg_ctx_t * ctx, enum cg_fill_rule_t rule);
```

设置填充规则。同时适用于填充和裁剪。

### cg_set_line_width

```c
void cg_set_line_width(struct cg_ctx_t * ctx, float width);
```

设置描边线宽。线宽受当前变换矩阵影响（缩放 CTM 会改变表观描边宽度）。

### cg_set_line_cap

```c
void cg_set_line_cap(struct cg_ctx_t * ctx, enum cg_line_cap_t cap);
```

设置开放子路径端点的绘制方式。

### cg_set_line_join

```c
void cg_set_line_join(struct cg_ctx_t * ctx, enum cg_line_join_t join);
```

设置连接线段之间拐角的绘制方式。

### cg_set_miter_limit

```c
void cg_set_miter_limit(struct cg_ctx_t * ctx, float limit);
```

设置斜接限制。当使用 `CG_LINE_JOIN_MITER` 连接线段时，如果斜接延伸将超过 `limit * line_width / 2`，连接将被转换为平角。

### cg_set_dash

```c
void cg_set_dash(struct cg_ctx_t * ctx, float * dashes, int ndashes, float offset);
```

设置虚线模式。`dashes` 是一个交替的实线/空白长度数组（用户空间单位）。如果数组有奇数个元素，实际上会被加倍。`offset` 指定从模式中的哪个位置开始。

要禁用虚线，将 `ndashes` 设为 `0`。

### cg_set_dash_array

```c
void cg_set_dash_array(struct cg_ctx_t * ctx, float * dashes, int ndashes);
```

仅设置虚线数组，保留当前偏移量。

### cg_set_dash_offset

```c
void cg_set_dash_offset(struct cg_ctx_t * ctx, float offset);
```

仅设置虚线偏移量，保留当前数组。

## 变换

所有变换函数将给定的变换后乘到当前变换矩阵（CTM）上。这意味着变换按调用顺序应用——最后调用的变换最先生效。

### cg_translate

```c
void cg_translate(struct cg_ctx_t * ctx, float tx, float ty);
```

将 CTM 平移 `(tx, ty)`。

### cg_scale

```c
void cg_scale(struct cg_ctx_t * ctx, float sx, float sy);
```

将 CTM 缩放 `(sx, sy)`。

### cg_shear

```c
void cg_shear(struct cg_ctx_t * ctx, float shx, float shy);
```

剪切 CTM。

### cg_rotate

```c
void cg_rotate(struct cg_ctx_t * ctx, float angle);
```

将 CTM 旋转 `angle` 弧度，顺时针方向。

### cg_transform

```c
void cg_transform(struct cg_ctx_t * ctx, struct cg_matrix_t * m);
```

将 CTM 后乘 `m`：`CTM = m * CTM`。

### cg_set_matrix

```c
void cg_set_matrix(struct cg_ctx_t * ctx, struct cg_matrix_t * m);
```

用 `m` 替换 CTM。

### cg_identity_matrix

```c
void cg_identity_matrix(struct cg_ctx_t * ctx);
```

将 CTM 重置为单位矩阵。

## 路径构建

这些函数构建当前路径。坐标在用户空间中，在渲染时会被 CTM 变换。

### 基本操作

#### cg_move_to

```c
void cg_move_to(struct cg_ctx_t * ctx, float x, float y);
```

在 `(x, y)` 处开始新的子路径。

#### cg_line_to

```c
void cg_line_to(struct cg_ctx_t * ctx, float x, float y);
```

添加一条到 `(x, y)` 的直线段。

#### cg_quad_to

```c
void cg_quad_to(struct cg_ctx_t * ctx, float x1, float y1, float x2, float y2);
```

添加二次贝塞尔曲线，控制点为 `(x1, y1)`，终点为 `(x2, y2)`。内部转换为三次。

#### cg_cubic_to

```c
void cg_cubic_to(struct cg_ctx_t * ctx,
    float x1, float y1,
    float x2, float y2,
    float x3, float y3);
```

添加三次贝塞尔曲线。

#### cg_arc_to

```c
void cg_arc_to(struct cg_ctx_t * ctx,
    float rx, float ry, float angle,
    int large, int sweep,
    float x, float y);
```

添加椭圆弧。详情参见 [cg_path_arc_to](./path#cg_path_arc_to)。

### 相对坐标操作

这些函数通过将增量加到当前点来计算目标坐标：

```c
void cg_rel_move_to(struct cg_ctx_t * ctx, float dx, float dy);
void cg_rel_line_to(struct cg_ctx_t * ctx, float dx, float dy);
void cg_rel_quad_to(struct cg_ctx_t * ctx, float dx1, float dy1, float dx2, float dy2);
void cg_rel_cubic_to(struct cg_ctx_t * ctx, float dx1, float dy1, float dx2, float dy2, float dx3, float dy3);
void cg_rel_arc_to(struct cg_ctx_t * ctx, float rx, float ry, float angle, int large, int sweep, float dx, float dy);
```

例如，`cg_rel_line_to(ctx, 10, 0)` 从当前点向右画 10 单位的直线。

### 预定义形状

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

### 路径管理

#### cg_new_path

```c
void cg_new_path(struct cg_ctx_t * ctx);
```

清除所有线段，将路径重置为空。

#### cg_new_sub_path

```c
void cg_new_sub_path(struct cg_ctx_t * ctx);
```

开始新的子路径。下一个绘图命令将从给定点开始。

#### cg_close_path

```c
void cg_close_path(struct cg_ctx_t * ctx);
```

用一条直线将当前子路径闭合回到其起点。

#### cg_add_path

```c
void cg_add_path(struct cg_ctx_t * ctx, struct cg_path_t * path);
```

将外部路径追加到当前路径。路径元素直接复制（无变换）。

## 渲染

### cg_clip

```c
void cg_clip(struct cg_ctx_t * ctx);
```

使用当前填充规则光栅化当前路径，将其设为裁剪区域，替换任何之前的裁剪区域。然后清除路径。后续绘制仅在裁剪区域内可见。

:::note
裁剪区域以 span 缓冲区的形式存储。首次裁剪调用时，路径被光栅化并存储。后续的裁剪调用与现有裁剪 span 取交集。
:::

### cg_clip_preserve

```c
void cg_clip_preserve(struct cg_ctx_t * ctx);
```

与 `cg_clip` 相同，但不清除路径（适用于裁剪后再填充+描边）。

### cg_reset_clip

```c
void cg_reset_clip(struct cg_ctx_t * ctx);
```

将裁剪区域重置为整个 surface。清除所有累积的裁剪 span 并禁用裁剪。

### cg_fill

```c
void cg_fill(struct cg_ctx_t * ctx);
```

使用当前填充规则光栅化当前路径，然后将 paint 混合到填充区域内的 surface 上（与活动裁剪区域取交集）。之后清除路径。

### cg_fill_preserve

```c
void cg_fill_preserve(struct cg_ctx_t * ctx);
```

与 `cg_fill` 相同，但保留路径以便后续操作（例如先填充再描边）。

### cg_mask

```c
void cg_mask(struct cg_ctx_t * ctx, struct cg_paint_t * paint);
```

一种绘制操作，用 `paint` 的 alpha 通道作为逐像素遮罩，在当前路径填充区域内绘制当前源。mask paint 的不透明区域将被源色填充，透明区域则不被绘制。完成后清除路径。

### cg_mask_surface

```c
void cg_mask_surface(struct cg_ctx_t * ctx, struct cg_surface_t * mask, float x, float y);
```

便捷封装函数，将 `mask` 包装为纹理 paint（定位在用户空间 `(x, y)`），然后调用 `cg_mask(ctx, paint)`。

### cg_stroke

```c
void cg_stroke(struct cg_ctx_t * ctx);
```

使用当前描边样式（线宽、线帽、连接、斜接限制、虚线模式）对当前路径进行描边。如果设置了虚线，路径首先被虚线化，然后使用底层 FreeType stroker 进行描边。清除路径。

### cg_stroke_preserve

```c
void cg_stroke_preserve(struct cg_ctx_t * ctx);
```

与 `cg_stroke` 相同，但保留路径。

### cg_paint

```c
void cg_paint(struct cg_ctx_t * ctx);
```

用当前 paint 填充整个裁剪区域（如果未设置裁剪，则为整个 surface）。不使用路径。通常用于用纹理或渐变覆盖整个区域。

### cg_paint_with_alpha

```c
void cg_paint_with_alpha(struct cg_ctx_t * ctx, float alpha);
```

用当前 paint 以指定的透明度值填充整个裁剪区域。等价于 `cg_save(ctx)` + `cg_set_opacity(ctx, alpha)` + `cg_paint(ctx)` + `cg_restore(ctx)`。调用后原有透明度设置会被保留。
