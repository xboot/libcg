# Paint Functions

Paint objects define "what to draw" — they can be a solid color, gradient, or texture. Paint objects are standalone and can be created once and reused across multiple drawing operations or contexts. Call `cg_paint_destroy` when done to release.

## Solid Color Paint

### cg_paint_create_rgb

```c
struct cg_paint_t * cg_paint_create_rgb(float r, float g, float b);
```

Creates a solid color paint with alpha = `1.0`. Equivalent to `cg_paint_create_rgba(r, g, b, 1.0f)`.

### cg_paint_create_rgba

```c
struct cg_paint_t * cg_paint_create_rgba(float r, float g, float b, float a);
```

Creates a solid color paint. All components are clamped to `[0.0, 1.0]`.

### cg_paint_create_color

```c
struct cg_paint_t * cg_paint_create_color(struct cg_color_t * color);
```

Creates a solid color paint from a `cg_color_t` struct. Equivalent to `cg_paint_create_rgba(color->r, color->g, color->b, color->a)`.

## Gradient Paint

### cg_paint_create_linear_gradient

```c
struct cg_paint_t * cg_paint_create_linear_gradient(
    float x1, float y1,
    float x2, float y2,
    enum cg_spread_method_t spread,
    struct cg_gradient_stop_t * stops,
    int nstops,
    struct cg_matrix_t * m);
```

Creates a linear gradient paint. The gradient transitions from `stops[0].color` at `(x1, y1)` to `stops[nstops-1].color` at `(x2, y2)`. Intermediate stops are interpolated between these endpoints.

| Parameter  | Description                                                     |
|-----------|-----------------------------------------------------------------|
| `x1, y1`  | Start point of the gradient line                                |
| `x2, y2`  | End point of the gradient line                                  |
| `spread`  | How to handle coordinates outside `[0, 1]` along the gradient  |
| `stops`   | Array of `nstops` color stops (offsets clamped and sorted)      |
| `m`       | Optional user-to-gradient transform matrix, or `NULL` for identity |

### cg_paint_create_radial_gradient

```c
struct cg_paint_t * cg_paint_create_radial_gradient(
    float cx0, float cy0, float r0,
    float cx1, float cy1, float r1,
    enum cg_spread_method_t spread,
    struct cg_gradient_stop_t * stops,
    int nstops,
    struct cg_matrix_t * m);
```

Creates a radial gradient paint transitioning from circle `(cx0, cy0, r0)` at offset `0` to circle `(cx1, cy1, r1)` at offset `1`.

:::note
The `values` array in the paint is stored as `{cx0, cy0, r0, cx1, cy1, r1}` (start circle first, then end circle).
:::

## Texture Paint

### cg_paint_create_texture

```c
struct cg_paint_t * cg_paint_create_texture(
    struct cg_surface_t * surface,
    enum cg_texture_type_t type,
    float opacity,
    struct cg_matrix_t * m);
```

Creates a texture paint. The surface is referenced (refcount incremented) and will be released when the paint is destroyed. `opacity` is clamped to `[0, 1]` and multiplied with the context's global opacity.

The optional matrix `m` (or identity if `NULL`) defines a user-to-texture-space transform.

## Lifecycle

### cg_paint_destroy

```c
void cg_paint_destroy(struct cg_paint_t * paint);
```

Decrements the reference count. When it reaches zero:
- For texture paints: calls `cg_surface_destroy` on the referenced surface.
- For all paints: frees the memory.

### cg_paint_reference

```c
struct cg_paint_t * cg_paint_reference(struct cg_paint_t * paint);
```

Increments the reference count and returns the paint. Safe to call with `NULL`.
