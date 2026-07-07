# Clipping

This example demonstrates using libcg's clipping feature to limit the drawing area.

## Circular Clipping

Use `cg_clip` to create a circular clipping region; subsequent drawing is only visible inside the circle:

![clip](/screenshots/clip.png)

```c
struct cg_surface_t * surface = cg_surface_create(256, 256);
struct cg_ctx_t * ctx = cg_create(surface);

/* Create a circular clipping region */
cg_arc(ctx, 128.0, 128.0, 76.8, 0, 2 * M_PI);
cg_clip(ctx);

/* Fill the background (only visible inside the circle) */
cg_new_path(ctx);
cg_rectangle(ctx, 0, 0, 256, 256);
cg_fill(ctx);

/* Draw diagonals (only visible inside the circle) */
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

## Image Clipping

Combine clipping with image drawing to achieve a circular avatar effect:

![clip_image](/screenshots/clip_image.png)

```c
struct cg_surface_t * surface = cg_surface_create(256, 256);
struct cg_ctx_t * ctx = cg_create(surface);

struct cg_surface_t * img = cg_surface_create_for_data(128, 128, (void *)cat_img_128_128);

/* Create a circular clipping region */
cg_arc(ctx, 128.0, 128.0, 76.8, 0, 2 * M_PI);
cg_clip(ctx);

/* Scale and draw the image (only visible inside the circle) */
cg_scale(ctx, 256.0 / img->width, 256.0 / img->height);
cg_set_source_surface(ctx, img, 0, 0);
cg_paint(ctx);
cg_surface_destroy(img);

cg_destroy(ctx);
cg_surface_destroy(surface);
```

## Clipping Operation Key Points

- `cg_clip` — sets the clipping region and clears the current path
- `cg_clip_preserve` — sets the clipping region but preserves the path (can continue to fill or stroke)
- `cg_reset_clip` — resets the clipping region to the entire surface

When `cg_clip` is called multiple times, the clipping regions are intersected:

```c
/* First clip: circle */
cg_arc(ctx, 128, 128, 100, 0, 2 * M_PI);
cg_clip(ctx);

/* Second clip: rectangle, intersected with the circle */
cg_rectangle(ctx, 50, 50, 156, 156);
cg_clip(ctx);

/* Subsequent drawing is only visible in the intersection of circle and rectangle */
```

:::note
The clipping region is stored as a span buffer. On the first clip call, the path is rasterized and stored. Subsequent clip calls intersect with the existing clip spans.
:::
