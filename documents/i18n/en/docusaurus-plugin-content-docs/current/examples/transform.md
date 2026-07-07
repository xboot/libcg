# Transformations

This example demonstrates how to use libcg's coordinate transformations (translation, rotation, scaling) to draw shapes.

## Image Transformation

Center the image, rotate it 45 degrees, and scale it to fill the entire surface:

![image](/screenshots/image.png)

```c
struct cg_surface_t * surface = cg_surface_create(256, 256);
struct cg_ctx_t * ctx = cg_create(surface);

struct cg_surface_t * img = cg_surface_create_for_data(128, 128, (void *)cat_img_128_128);

/* Transformation order: translate to center, rotate, scale, then translate back to image center */
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

## Transformation Order

libcg's transformation functions post-multiply the current transformation matrix (CTM), so transformations are applied in call order — **the last transformation called takes effect first**.

For example, the following code:

```c
cg_translate(ctx, 128, 128);   /* 3. First translate to center */
cg_rotate(ctx, M_PI / 4);      /* 2. Then rotate */
cg_scale(ctx, 2.0, 1.0);       /* 1. Scale first */
cg_rectangle(ctx, -30, -20, 60, 40);
cg_fill(ctx);
```

The actual effect is: the rectangle is first stretched 2x along the X axis at the origin, then rotated 45 degrees, and finally translated to (128, 128).

## Save/Restore to Isolate Transformations

Use `cg_save` / `cg_restore` to isolate transformations in different regions:

```c
cg_set_source_rgb(ctx, 1, 0, 0);

/* First rectangle: rotation */
cg_save(ctx);
cg_translate(ctx, 64, 64);
cg_rotate(ctx, M_PI / 4);
cg_rectangle(ctx, -30, -20, 60, 40);
cg_fill(ctx);
cg_restore(ctx);

/* Second rectangle: scaling */
cg_save(ctx);
cg_translate(ctx, 192, 192);
cg_scale(ctx, 2.0, 1.0);
cg_rectangle(ctx, -30, -20, 60, 40);
cg_fill(ctx);
cg_restore(ctx);
```

After `cg_restore`, the CTM reverts to the state at the time of `cg_save`, not affecting subsequent drawing.

## Taiji Diagram

Draw a Taiji diagram using arcs and fills, showcasing the combination of transformations and paths:

![taiji](/screenshots/taiji.png)

```c
/* Black semicircle */
cg_arc(ctx, 128.0, 128.0, 110.0, -0.5 * M_PI, 0.5 * M_PI);
cg_arc(ctx, 128.0, 183.0, 55.0, 0.5 * M_PI, 1.5 * M_PI);
cg_arc_negative(ctx, 128.0, 73.0, 55.0, 0.5 * M_PI, -0.5 * M_PI);
cg_close_path(ctx);
cg_set_source_rgb(ctx, 0, 0, 0);
cg_fill(ctx);

/* White semicircle */
cg_arc(ctx, 128.0, 128.0, 110.0, 0.5 * M_PI, 1.5 * M_PI);
cg_arc(ctx, 128.0, 73.0, 55.0, 1.5 * M_PI, 0.5 * M_PI);
cg_arc_negative(ctx, 128.0, 183.0, 55.0, 1.5 * M_PI, 0.5 * M_PI);
cg_close_path(ctx);
cg_set_source_rgb(ctx, 1, 1, 1);
cg_fill(ctx);

/* Small black and white circles */
cg_arc(ctx, 128.0, 73.0, 13.75, 0, 2 * M_PI);
cg_set_source_rgb(ctx, 0, 0, 0);
cg_fill(ctx);

cg_arc(ctx, 128.0, 183.0, 13.75, 0, 2 * M_PI);
cg_set_source_rgb(ctx, 1, 1, 1);
cg_fill(ctx);

/* Outer circle stroke */
cg_arc(ctx, 128.0, 128.0, 110.0, 0, 2 * M_PI);
cg_set_source_rgb(ctx, 0, 0, 0);
cg_set_line_width(ctx, 5.0);
cg_stroke(ctx);
```

## Logo

Draw a four-color logo using multi-color fills:

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

/* The other three quadrants are similar, using different colors... */
```
