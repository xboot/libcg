# Gradients

This example demonstrates drawing linear and radial gradients with libcg.

## Linear + Radial Gradient Combination

The top half is a linear gradient background, with a radial gradient circle overlaid in the center:

![gradient](/screenshots/gradient.png)

```c
struct cg_surface_t * surface = cg_surface_create(256, 256);
struct cg_ctx_t * ctx = cg_create(surface);

/* Linear gradient background: white to black */
struct cg_gradient_stop_t linear_stops[] = {
    { 0.0, { 1, 1, 1, 1 } },
    { 1.0, { 0, 0, 0, 1 } },
};
cg_rectangle(ctx, 0, 0, 256, 256);
cg_set_linear_gradient(ctx, 0.0, 0.0, 0.0, 256.0,
    CG_SPREAD_METHOD_PAD, linear_stops, 2, NULL);
cg_fill(ctx);

/* Radial gradient circle: white to black */
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

## Spread Methods

The gradient's `spread` parameter controls how colors outside the `[0, 1]` range are sampled:

| Mode | Effect |
|------|--------|
| `CG_SPREAD_METHOD_PAD` | Clamped to the nearest edge color |
| `CG_SPREAD_METHOD_REFLECT` | Mirrored at the boundary |
| `CG_SPREAD_METHOD_REPEAT` | Repeated/tiled |

## Multi-stop Gradient

Use multiple color stops to create a rainbow effect:

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

## Rainbow

Draw a rainbow by filling multiple arcs:

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
