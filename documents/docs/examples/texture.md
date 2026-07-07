# 纹理绘制

本示例演示使用 libcg 绘制纹理（图像），包括平铺纹理与变换组合。

## 平铺纹理

使用 `CG_TEXTURE_TYPE_TILED` 将纹理重复填充整个区域，配合旋转变换：

![texture_tiled](/screenshots/texture_tiled.png)

```c
struct cg_surface_t * surface = cg_surface_create(256, 256);
struct cg_ctx_t * ctx = cg_create(surface);

struct cg_surface_t * img = cg_surface_create_for_data(128, 128, (void *)cat_img_128_128);

/* 变换：平移到中心，旋转 -45 度，缩放 0.8，平移回图像中心 */
cg_translate(ctx, 128.0, 128.0);
cg_rotate(ctx, -45 * M_PI / 180);
cg_scale(ctx, 0.8, 0.8);
cg_translate(ctx, -0.5 * img->width, -0.5 * img->height);

/* 设置平铺纹理并填充 */
cg_set_texture(ctx, img, CG_TEXTURE_TYPE_TILED, 1.0, NULL);
cg_paint(ctx);
cg_surface_destroy(img);

cg_destroy(ctx);
cg_surface_destroy(surface);
```

## 纹理类型

| 类型 | 效果 |
|------|------|
| `CG_TEXTURE_TYPE_PLAIN` | 单次绘制纹理（区域外为透明） |
| `CG_TEXTURE_TYPE_TILED` | 纹理重复以填充整个绘制区域 |

:::note
经过变换的非平铺纹理，以及带有旋转/剪切的平铺纹理，使用双线性过滤。其他组合使用最近邻采样。
:::

## 普通纹理

使用 `cg_set_source_surface` 绘制单次纹理（内部创建 `PLAIN` 类型纹理 paint）：

```c
struct cg_surface_t * img = cg_surface_create_for_data(128, 128, pixels);

/* 在 (64, 64) 位置绘制图像 */
cg_set_source_surface(ctx, img, 64, 64);
cg_paint(ctx);
```

## 带透明度的纹理

通过 `cg_set_texture` 的 `opacity` 参数控制纹理透明度：

```c
/* 50% 透明度的平铺纹理 */
cg_set_texture(ctx, img, CG_TEXTURE_TYPE_TILED, 0.5, NULL);
cg_paint(ctx);
```

`opacity` 会与上下文的全局不透明度（`cg_set_opacity`）相乘。
