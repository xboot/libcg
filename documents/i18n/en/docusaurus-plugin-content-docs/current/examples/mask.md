# Masking

This example demonstrates libcg's masking feature, which controls the visible drawing region via the alpha channel.

## Surface Mask

Use `cg_mask_surface` to use a surface's alpha channel as a mask, achieving a gradient fade-in effect:

![mask_surface](/screenshots/mask_surface.png)

```c
struct cg_surface_t * surface = cg_surface_create(256, 256);
struct cg_ctx_t * ctx = cg_create(surface);

/* Draw a green gradient background */
struct cg_gradient_stop_t bg_stops[] = {
    { 0.0, { 0.75, 0.95, 0.78, 1 } },
    { 1.0, { 0.60, 0.85, 0.65, 1 } },
};
cg_set_linear_gradient(ctx, 0, 0, 0, 256, CG_SPREAD_METHOD_PAD, bg_stops, 2, NULL);
cg_rectangle(ctx, 0, 0, 256, 256);
cg_fill(ctx);

/* Create the mask surface: a radial gradient from transparent center to opaque edge */
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

/* Set the source image */
struct cg_surface_t * cat = cg_surface_create_for_data(128, 128, (void *)cat_img_128_128);
cg_set_source_surface(ctx, cat, 64, 64);
cg_surface_destroy(cat);

/* Apply the mask: the source image is only drawn where the mask alpha > 0 */
cg_arc(ctx, 128, 128, 64, 0, 2 * M_PI);
cg_mask_surface(ctx, mask, 64, 64);

cg_surface_destroy(mask);
cg_destroy(ctx);
cg_surface_destroy(surface);
```

## cg_mask vs cg_mask_surface

| Function | Parameters | Description |
|----------|------------|-------------|
| `cg_mask` | `paint` | Uses the alpha channel of a paint object as the mask |
| `cg_mask_surface` | `surface, x, y` | Convenience wrapper that wraps the surface as a texture paint and then calls `cg_mask` |

`cg_mask_surface(ctx, mask, x, y)` is equivalent to:

```c
struct cg_paint_t * paint = cg_paint_create_texture(mask, CG_TEXTURE_TYPE_PLAIN, 1.0, &translate_matrix);
cg_mask(ctx, paint);
cg_paint_destroy(paint);
```

:::note
The mask operation uses the mask's alpha channel as per-pixel coverage. The opaque regions of the mask will draw the source content, while transparent regions will not. After the operation, the current path is cleared.
:::
