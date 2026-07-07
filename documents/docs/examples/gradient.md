# 渐变绘制

本示例演示使用 libcg 绘制线性渐变和径向渐变。

## 线性渐变 + 径向渐变组合

上半部分为线性渐变背景，中央叠加一个径向渐变圆：

![gradient](/screenshots/gradient.png)

```c
struct cg_surface_t * surface = cg_surface_create(256, 256);
struct cg_ctx_t * ctx = cg_create(surface);

/* 线性渐变背景：从白色到黑色 */
struct cg_gradient_stop_t linear_stops[] = {
    { 0.0, { 1, 1, 1, 1 } },
    { 1.0, { 0, 0, 0, 1 } },
};
cg_rectangle(ctx, 0, 0, 256, 256);
cg_set_linear_gradient(ctx, 0.0, 0.0, 0.0, 256.0,
    CG_SPREAD_METHOD_PAD, linear_stops, 2, NULL);
cg_fill(ctx);

/* 径向渐变圆：从白色到黑色 */
struct cg_gradient_stop_t radial_stops[] = {
    { 0.0, { 1, 1, 1, 1 } },
    { 1.0, { 0, 0, 0, 1 } },
};
cg_set_radial_gradient(ctx, 115.2, 102.4, 25.6, 102.4, 102.4, 128.0,
    CG_SPREAD_METHOD_PAD, radial_stops, 2, NULL);
cg_arc(ctx, 128.0, 128.0, 76.8, 0, 2 * M_PI);
cg_fill(ctx);

cg_destroy(ctx);
cg_surface_destroy(surface);
```

## 扩展模式

渐变的 `spread` 参数控制 `[0, 1]` 范围外的颜色如何采样：

| 模式 | 效果 |
|------|------|
| `CG_SPREAD_METHOD_PAD` | 钳制到最近的边缘颜色 |
| `CG_SPREAD_METHOD_REFLECT` | 在边界处镜像翻转 |
| `CG_SPREAD_METHOD_REPEAT` | 重复环绕 |

## 多色标渐变

使用多个色标创建彩虹效果：

```c
struct cg_gradient_stop_t stops[] = {
    { 0.0,  { 1, 0, 0, 1 } },
    { 0.2,  { 1, 1, 0, 1 } },
    { 0.4,  { 0, 1, 0, 1 } },
    { 0.6,  { 0, 1, 1, 1 } },
    { 0.8,  { 0, 0, 1, 1 } },
    { 1.0,  { 1, 0, 1, 1 } },
};

cg_rectangle(ctx, 0, 0, 256, 256);
cg_set_linear_gradient(ctx, 0, 0, 256, 0,
    CG_SPREAD_METHOD_PAD, stops, 6, NULL);
cg_fill(ctx);
```

## 彩虹

使用多个圆弧填充绘制彩虹：

![rainbow](/screenshots/rainbow.png)

```c
float cx = 128.0, cy = 210.0;
float colors[7][3] = {
    { 0.92, 0.18, 0.18 },
    { 0.98, 0.48, 0.15 },
    { 0.99, 0.84, 0.15 },
    { 0.15, 0.78, 0.28 },
    { 0.15, 0.45, 0.92 },
    { 0.25, 0.20, 0.80 },
    { 0.50, 0.15, 0.70 },
};
for(int band = 0; band < 7; band++)
{
    float r1 = 115.0f - band * 9.0f;
    float r2 = r1 - 9.5f;
    cg_new_path(ctx);
    cg_arc(ctx, cx, cy, r1, (float)M_PI, 0);
    cg_arc_negative(ctx, cx, cy, r2, 0, (float)M_PI);
    cg_close_path(ctx);
    cg_set_source_rgb(ctx, colors[band][0], colors[band][1], colors[band][2]);
    cg_fill(ctx);
}
```
