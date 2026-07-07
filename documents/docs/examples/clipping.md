# 裁剪

本示例演示使用 libcg 的裁剪功能限制绘制区域。

## 圆形裁剪

使用 `cg_clip` 创建圆形裁剪区域，后续绘制仅在圆内可见：

![clip](/screenshots/clip.png)

```c
struct cg_surface_t * surface = cg_surface_create(256, 256);
struct cg_ctx_t * ctx = cg_create(surface);

/* 创建圆形裁剪区域 */
cg_arc(ctx, 128.0, 128.0, 76.8, 0, 2 * M_PI);
cg_clip(ctx);

/* 填充背景（仅圆内可见） */
cg_new_path(ctx);
cg_rectangle(ctx, 0, 0, 256, 256);
cg_fill(ctx);

/* 绘制对角线（仅圆内可见） */
cg_set_source_rgb(ctx, 0, 1, 0);
cg_move_to(ctx, 0, 0);
cg_line_to(ctx, 256, 256);
cg_move_to(ctx, 256, 0);
cg_line_to(ctx, 0, 256);
cg_set_line_width(ctx, 10.0);
cg_stroke(ctx);

cg_destroy(ctx);
cg_surface_destroy(surface);
```

## 图像裁剪

将裁剪与图像绘制结合，实现圆形头像效果：

![clip_image](/screenshots/clip_image.png)

```c
struct cg_surface_t * surface = cg_surface_create(256, 256);
struct cg_ctx_t * ctx = cg_create(surface);

struct cg_surface_t * img = cg_surface_create_for_data(128, 128, (void *)cat_img_128_128);

/* 创建圆形裁剪区域 */
cg_arc(ctx, 128.0, 128.0, 76.8, 0, 2 * M_PI);
cg_clip(ctx);

/* 缩放并绘制图像（仅圆内可见） */
cg_scale(ctx, 256.0 / img->width, 256.0 / img->height);
cg_set_source_surface(ctx, img, 0, 0);
cg_paint(ctx);
cg_surface_destroy(img);

cg_destroy(ctx);
cg_surface_destroy(surface);
```

## 裁剪操作要点

- `cg_clip` — 设置裁剪区域并清除当前路径
- `cg_clip_preserve` — 设置裁剪区域但保留路径（可继续填充或描边）
- `cg_reset_clip` — 重置裁剪区域为整个 surface

多次调用 `cg_clip` 时，裁剪区域会取交集：

```c
/* 第一个裁剪：圆形 */
cg_arc(ctx, 128, 128, 100, 0, 2 * M_PI);
cg_clip(ctx);

/* 第二个裁剪：矩形，与圆形取交集 */
cg_rectangle(ctx, 50, 50, 156, 156);
cg_clip(ctx);

/* 后续绘制仅在圆与矩形的交集内可见 */
```

:::note
裁剪区域以 span 缓冲区的形式存储。首次裁剪调用时，路径被光栅化并存储。后续的裁剪调用与现有裁剪 span 取交集。
:::
