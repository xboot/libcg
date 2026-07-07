# 变换

本示例演示如何使用 libcg 的坐标变换（平移、旋转、缩放）绘制图形。

## 图像变换

将图像居中后旋转 45 度并缩放填充整个 surface：

![image](/screenshots/image.png)

```c
struct cg_surface_t * surface = cg_surface_create(256, 256);
struct cg_ctx_t * ctx = cg_create(surface);

struct cg_surface_t * img = cg_surface_create_for_data(128, 128, (void *)cat_img_128_128);

/* 变换顺序：先平移到中心，再旋转，再缩放，再平移回图像中心 */
cg_translate(ctx, 128.0, 128.0);
cg_rotate(ctx, 45 * M_PI / 180);
cg_scale(ctx, 256.0 / img->width, 256.0 / img->height);
cg_translate(ctx, -0.5 * img->width, -0.5 * img->height);

cg_set_source_surface(ctx, img, 0, 0);
cg_paint(ctx);
cg_surface_destroy(img);

cg_destroy(ctx);
cg_surface_destroy(surface);
```

## 变换顺序

libcg 的变换函数将变换后乘到当前变换矩阵（CTM）上，因此变换按调用顺序应用——**最后调用的变换最先生效**。

例如，以下代码：

```c
cg_translate(ctx, 128, 128);   /* 3. 先平移到中心 */
cg_rotate(ctx, M_PI / 4);      /* 2. 再旋转 */
cg_scale(ctx, 2.0, 1.0);       /* 1. 先缩放 */
cg_rectangle(ctx, -30, -20, 60, 40);
cg_fill(ctx);
```

实际效果是：先将矩形在原点处沿 X 方向拉伸 2 倍，再旋转 45 度，最后平移到 (128, 128)。

## Save/Restore 隔离变换

使用 `cg_save` / `cg_restore` 隔离不同区域的变换：

```c
cg_set_source_rgb(ctx, 1, 0, 0);

/* 第一个矩形：旋转 */
cg_save(ctx);
cg_translate(ctx, 64, 64);
cg_rotate(ctx, M_PI / 4);
cg_rectangle(ctx, -30, -20, 60, 40);
cg_fill(ctx);
cg_restore(ctx);

/* 第二个矩形：缩放 */
cg_save(ctx);
cg_translate(ctx, 192, 192);
cg_scale(ctx, 2.0, 1.0);
cg_rectangle(ctx, -30, -20, 60, 40);
cg_fill(ctx);
cg_restore(ctx);
```

`cg_restore` 后，CTM 恢复到 `cg_save` 时的状态，不影响后续绘制。

## 太极图

使用圆弧和填充绘制太极图，展示变换与路径的组合：

![taiji](/screenshots/taiji.png)

```c
/* 黑色半圆 */
cg_arc(ctx, 128.0, 128.0, 110.0, -0.5 * M_PI, 0.5 * M_PI);
cg_arc(ctx, 128.0, 183.0, 55.0, 0.5 * M_PI, 1.5 * M_PI);
cg_arc_negative(ctx, 128.0, 73.0, 55.0, 0.5 * M_PI, -0.5 * M_PI);
cg_close_path(ctx);
cg_set_source_rgb(ctx, 0, 0, 0);
cg_fill(ctx);

/* 白色半圆 */
cg_arc(ctx, 128.0, 128.0, 110.0, 0.5 * M_PI, 1.5 * M_PI);
cg_arc(ctx, 128.0, 73.0, 55.0, 1.5 * M_PI, 0.5 * M_PI);
cg_arc_negative(ctx, 128.0, 183.0, 55.0, 1.5 * M_PI, 0.5 * M_PI);
cg_close_path(ctx);
cg_set_source_rgb(ctx, 1, 1, 1);
cg_fill(ctx);

/* 黑白小圆 */
cg_arc(ctx, 128.0, 73.0, 13.75, 0, 2 * M_PI);
cg_set_source_rgb(ctx, 0, 0, 0);
cg_fill(ctx);

cg_arc(ctx, 128.0, 183.0, 13.75, 0, 2 * M_PI);
cg_set_source_rgb(ctx, 1, 1, 1);
cg_fill(ctx);

/* 外圆描边 */
cg_arc(ctx, 128.0, 128.0, 110.0, 0, 2 * M_PI);
cg_set_source_rgb(ctx, 0, 0, 0);
cg_set_line_width(ctx, 5.0);
cg_stroke(ctx);
```

## Logo

使用多色填充绘制四色 logo：

![logo](/screenshots/logo.png)

```c
cg_set_source_rgba(ctx, 0.914, 0.243, 0.188, 1.0);
cg_move_to(ctx, 33.2, 0);
cg_line_to(ctx, 0, 33.2);
cg_line_to(ctx, 90.9, 124.1);
cg_line_to(ctx, 124.1, 124.1);
cg_line_to(ctx, 124.1, 90.9);
cg_close_path(ctx);
cg_fill(ctx);

/* 其他三个象限类似，使用不同颜色... */
```
