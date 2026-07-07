# Textures

This example demonstrates drawing textures (images) with libcg, including tiled textures and transformations.

## Tiled Texture

Use `CG_TEXTURE_TYPE_TILED` to repeat the texture to fill the entire area, combined with a rotation transform:

![texture_tiled](/screenshots/texture_tiled.png)

```c
struct cg_surface_t * surface = cg_surface_create(256, 256);
struct cg_ctx_t * ctx = cg_create(surface);

struct cg_surface_t * img = cg_surface_create_for_data(128, 128, (void *)cat_img_128_128);

/* Transform: translate to center, rotate -45 degrees, scale 0.8, translate back to image center */
cg_translate(ctx, 128.0, 128.0);
cg_rotate(ctx, -45 * M_PI / 180);
cg_scale(ctx, 0.8, 0.8);
cg_translate(ctx, -0.5 * img->width, -0.5 * img->height);

/* Set a tiled texture and fill */
cg_set_texture(ctx, img, CG_TEXTURE_TYPE_TILED, 1.0, NULL);
cg_paint(ctx);
cg_surface_destroy(img);

cg_destroy(ctx);
cg_surface_destroy(surface);
```

## Texture Types

| Type | Effect |
|------|--------|
| `CG_TEXTURE_TYPE_PLAIN` | Draw the texture once (transparent outside the region) |
| `CG_TEXTURE_TYPE_TILED` | Texture repeated to fill the entire drawing region |

:::note
Transformed non-tiled textures, and tiled textures with rotation/shear, use bilinear filtering. Other combinations use nearest-neighbor sampling.
:::

## Plain Texture

Use `cg_set_source_surface` to draw a single texture (a `PLAIN` type texture paint is created internally):

```c
struct cg_surface_t * img = cg_surface_create_for_data(128, 128, pixels);

/* Draw the image at position (64, 64) */
cg_set_source_surface(ctx, img, 64, 64);
cg_paint(ctx);
```

## Texture with Opacity

Control the texture opacity via the `opacity` parameter of `cg_set_texture`:

```c
/* Tiled texture with 50% opacity */
cg_set_texture(ctx, img, CG_TEXTURE_TYPE_TILED, 0.5, NULL);
cg_paint(ctx);
```

`opacity` is multiplied with the context's global opacity (`cg_set_opacity`).
