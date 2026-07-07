# 遮罩

本示例演示使用 libcg 的遮罩功能，通过 alpha 通道控制绘制的可见区域。

## Surface 遮罩

使用 `cg_mask_surface` 将一个 surface 的 alpha 通道作为遮罩，实现渐变淡入效果：

![mask_surface](/screenshots/mask_surface.png)

```c
struct cg_surface_t * surface = cg_surface_create(256, 256);
struct cg_ctx_t * ctx = cg_create(surface);

/* 绘制绿色渐变背景 */
struct cg_gradient_stop_t bg_stops[] = {
    { 0.0, { 0.75, 0.95, 0.78, 1 } },
    { 1.0, { 0.60, 0.85, 0.65, 1 } },
};
cg_set_linear_gradient(ctx, 0, 0, 0, 256, CG_SPREAD_METHOD_PAD, bg_stops, 2, NULL);
cg_rectangle(ctx, 0, 0, 256, 256);
cg_fill(ctx);

/* 创建遮罩 surface：径向渐变从中心透明到边缘不透明 */
struct cg_surface_t * mask = cg_surface_create(128, 128);
struct cg_ctx_t * mctx = cg_create(mask);
struct cg_gradient_stop_t mstops[] = {
    { 0.0, { 1, 1, 1, 1 } },
    { 1.0, { 1, 1, 1, 0 } },
};
cg_set_radial_gradient(mctx, 64, 64, 0, 64, 64, 64, CG_SPREAD_METHOD_PAD, mstops, 2, NULL);
cg_rectangle(mctx, 0, 0, 128, 128);
cg_fill(mctx);
cg_destroy(mctx);

/* 设置源图像 */
struct cg_surface_t * cat = cg_surface_create_for_data(128, 128, (void *)cat_img_128_128);
cg_set_source_surface(ctx, cat, 64, 64);
cg_surface_destroy(cat);

/* 应用遮罩：仅遮罩 alpha > 0 的区域显示源图像 */
cg_arc(ctx, 128, 128, 64, 0, 2 * M_PI);
cg_mask_surface(ctx, mask, 64, 64);

cg_surface_destroy(mask);
cg_destroy(ctx);
cg_surface_destroy(surface);
```

## cg_mask vs cg_mask_surface

| 函数 | 参数 | 说明 |
|------|------|------|
| `cg_mask` | `paint` | 使用 paint 对象的 alpha 通道作为遮罩 |
| `cg_mask_surface` | `surface, x, y` | 便捷封装，将 surface 包装为纹理 paint 后调用 `cg_mask` |

`cg_mask_surface(ctx, mask, x, y)` 等价于：

```c
struct cg_paint_t * paint = cg_paint_create_texture(mask, CG_TEXTURE_TYPE_PLAIN, 1.0, &translate_matrix);
cg_mask(ctx, paint);
cg_paint_destroy(paint);
```

:::note
遮罩操作使用 mask 的 alpha 通道作为逐像素覆盖率。mask 的不透明区域将绘制源内容，透明区域则不绘制。操作完成后清除当前路径。
:::
