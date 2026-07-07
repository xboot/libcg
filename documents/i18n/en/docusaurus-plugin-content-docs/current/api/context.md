# Context Functions

The drawing context `cg_ctx_t` is the central object of libcg; all drawing operations are performed through the context. It holds a reference to the target surface, the current path, and a state stack.

## Creation and Destruction

### cg_create

```c
struct cg_ctx_t * cg_create(struct cg_surface_t * surface);
```

Creates a drawing context bound to `surface`. The surface is referenced. Initializes a default state, an empty path, and sets the clip rectangle to cover the entire surface `{0, 0, width, height}`.

### cg_destroy

```c
void cg_destroy(struct cg_ctx_t * ctx);
```

Destroys the context. Frees all saved states, the freed-state pool, span buffers, the surface reference, and the path. Safe to call with `NULL`.

## State Management

Drawing states are stored on a linked-list stack. `cg_save` pushes a copy of the current state; `cg_restore` pops it. States include: paint, color, matrix, stroke style (width/cap/join/miter/dash), fill rule, operator, opacity, and clip region.

The library maintains a freed-state pool to avoid repeated `malloc`/`free` during save/restore cycles.

### cg_save

```c
void cg_save(struct cg_ctx_t * ctx);
```

Pushes the current state onto the stack. All subsequent modifications to paint, matrix, stroke style, etc. are isolated until `cg_restore` is called.

### cg_restore

```c
void cg_restore(struct cg_ctx_t * ctx);
```

Pops the top state from the stack and restores it. Does nothing if only one state remains (the initial state is never popped).

## Query Functions

### cg_get_source

```c
struct cg_paint_t * cg_get_source(struct cg_ctx_t * ctx, struct cg_color_t * color);
```

Returns the current paint object (may be `NULL` if using simple color mode). If `color` is non-`NULL`, it is filled from the color stored in the state.

:::note
When you call `cg_set_source_rgba`, it sets `state->color` to the given RGBA and sets `state->paint = NULL`. So `cg_get_source` would return `NULL`, but `cg_get_source(ctx, &c)` would fill `c` with the correct color.
:::

### cg_get_surface

```c
struct cg_surface_t * cg_get_surface(struct cg_ctx_t * ctx);
```

Returns the surface bound to this context.

### cg_get_path

```c
struct cg_path_t * cg_get_path(struct cg_ctx_t * ctx);
```

Returns the current path being constructed.

### cg_get_matrix

```c
struct cg_matrix_t * cg_get_matrix(struct cg_ctx_t * ctx);
```

Returns a pointer to the current transformation matrix (mutable — changes affect the context).

### cg_get_operator

```c
enum cg_operator_t cg_get_operator(struct cg_ctx_t * ctx);
```

Returns the current compositing operator.

### cg_get_opacity

```c
float cg_get_opacity(struct cg_ctx_t * ctx);
```

Returns the current global opacity.

### cg_get_current_point

```c
void cg_get_current_point(struct cg_ctx_t * ctx, float * x, float * y);
```

Returns the current path position.

### cg_has_current_point

```c
int cg_has_current_point(struct cg_ctx_t * ctx);
```

Checks whether the current path has a current point. Returns 1 if there is a current point, 0 otherwise.

## Spatial Queries

These functions rasterize the current path to determine spatial relationships. They can be potentially expensive if called repeatedly in a loop.

### cg_fill_extents

```c
void cg_fill_extents(struct cg_ctx_t * ctx, struct cg_rect_t * extents);
```

Rasterizes the current path using the current fill rule and writes the bounding box to `extents`.

### cg_stroke_extents

```c
void cg_stroke_extents(struct cg_ctx_t * ctx, struct cg_rect_t * extents);
```

Rasterizes the current path using the current stroke style and writes the bounding box to `extents`.

### cg_clip_extents

```c
void cg_clip_extents(struct cg_ctx_t * ctx, struct cg_rect_t * extents);
```

Returns the bounding box of the current clip region.

### cg_in_fill

```c
int cg_in_fill(struct cg_ctx_t * ctx, float x, float y);
```

Tests if the point `(x, y)` is inside the filled area of the current path.

### cg_in_stroke

```c
int cg_in_stroke(struct cg_ctx_t * ctx, float x, float y);
```

Tests if the point `(x, y)` is inside the stroked area of the current path.

### cg_in_clip

```c
int cg_in_clip(struct cg_ctx_t * ctx, float x, float y);
```

Tests if the point `(x, y)` is inside the current clip region.

## Paint and Style

### cg_set_source

```c
void cg_set_source(struct cg_ctx_t * ctx, struct cg_paint_t * paint);
```

Sets the current paint. The paint is referenced. Passing `NULL` reverts to using the color stored in the state.

### cg_set_source_rgb

```c
void cg_set_source_rgb(struct cg_ctx_t * ctx, float r, float g, float b);
```

Sets the paint to a solid RGB color (alpha = 1.0).

### cg_set_source_rgba

```c
void cg_set_source_rgba(struct cg_ctx_t * ctx, float r, float g, float b, float a);
```

Sets the paint to a solid RGBA color.

### cg_set_source_surface

```c
void cg_set_source_surface(struct cg_ctx_t * ctx, struct cg_surface_t * surface, float x, float y);
```

Creates a plain (non-tiled) texture paint from `surface` with a translation offset of `(x, y)`. Internally equivalent to:

```c
cg_set_texture(ctx, surface, CG_TEXTURE_TYPE_PLAIN, 1.0, &translate_matrix);
```

### cg_set_linear_gradient

```c
void cg_set_linear_gradient(struct cg_ctx_t * ctx,
    float x1, float y1, float x2, float y2,
    enum cg_spread_method_t spread,
    struct cg_gradient_stop_t * stops, int nstops,
    struct cg_matrix_t * m);
```

Creates and sets a linear gradient paint. See [cg_paint_create_linear_gradient](./paint#cg_paint_create_linear_gradient) for parameter details.

### cg_set_radial_gradient

```c
void cg_set_radial_gradient(struct cg_ctx_t * ctx,
    float cx0, float cy0, float r0,
    float cx1, float cy1, float r1,
    enum cg_spread_method_t spread,
    struct cg_gradient_stop_t * stops, int nstops,
    struct cg_matrix_t * m);
```

Creates and sets a radial gradient paint. See [cg_paint_create_radial_gradient](./paint#cg_paint_create_radial_gradient) for parameter details.

### cg_set_texture

```c
void cg_set_texture(struct cg_ctx_t * ctx,
    struct cg_surface_t * surface,
    enum cg_texture_type_t type,
    float opacity,
    struct cg_matrix_t * m);
```

Creates and sets a texture paint. See [cg_paint_create_texture](./paint#cg_paint_create_texture) for parameter details.

### cg_set_operator

```c
void cg_set_operator(struct cg_ctx_t * ctx, enum cg_operator_t op);
```

Sets the compositing operator used when blending onto the surface.

### cg_set_opacity

```c
void cg_set_opacity(struct cg_ctx_t * ctx, float opacity);
```

Sets the global opacity. Clamped to `[0.0, 1.0]`. This value is multiplied with the paint's opacity and per-span coverage.

### cg_set_fill_rule

```c
void cg_set_fill_rule(struct cg_ctx_t * ctx, enum cg_fill_rule_t rule);
```

Sets the fill rule. Applies to both filling and clipping.

### cg_set_line_width

```c
void cg_set_line_width(struct cg_ctx_t * ctx, float width);
```

Sets the stroke line width. The line width is affected by the current transformation matrix (scaling the CTM will change the apparent stroke width).

### cg_set_line_cap

```c
void cg_set_line_cap(struct cg_ctx_t * ctx, enum cg_line_cap_t cap);
```

Sets how the ends of open sub-paths are drawn.

### cg_set_line_join

```c
void cg_set_line_join(struct cg_ctx_t * ctx, enum cg_line_join_t join);
```

Sets how corners between connected line segments are drawn.

### cg_set_miter_limit

```c
void cg_set_miter_limit(struct cg_ctx_t * ctx, float limit);
```

Sets the miter limit. When joining lines with `CG_LINE_JOIN_MITER`, if the miter would extend beyond `limit * line_width / 2`, the join is converted to a bevel.

### cg_set_dash

```c
void cg_set_dash(struct cg_ctx_t * ctx, float * dashes, int ndashes, float offset);
```

Sets the dash pattern. `dashes` is an array of alternating dash/gap lengths (in user space units). If the array has an odd number of elements, it is effectively doubled. `offset` specifies where in the pattern to start.

To disable dashing, set `ndashes` to `0`.

### cg_set_dash_array

```c
void cg_set_dash_array(struct cg_ctx_t * ctx, float * dashes, int ndashes);
```

Sets only the dash array, preserving the current offset.

### cg_set_dash_offset

```c
void cg_set_dash_offset(struct cg_ctx_t * ctx, float offset);
```

Sets only the dash offset, preserving the current array.

## Transformations

All transform functions post-multiply the given transform onto the current transformation matrix (CTM). This means transforms are applied in the order they are called — the last transform called takes effect first.

### cg_translate

```c
void cg_translate(struct cg_ctx_t * ctx, float tx, float ty);
```

Translates the CTM by `(tx, ty)`.

### cg_scale

```c
void cg_scale(struct cg_ctx_t * ctx, float sx, float sy);
```

Scales the CTM by `(sx, sy)`.

### cg_shear

```c
void cg_shear(struct cg_ctx_t * ctx, float shx, float shy);
```

Shears the CTM.

### cg_rotate

```c
void cg_rotate(struct cg_ctx_t * ctx, float angle);
```

Rotates the CTM by `angle` radians, clockwise.

### cg_transform

```c
void cg_transform(struct cg_ctx_t * ctx, struct cg_matrix_t * m);
```

Post-multiplies the CTM by `m`: `CTM = m * CTM`.

### cg_set_matrix

```c
void cg_set_matrix(struct cg_ctx_t * ctx, struct cg_matrix_t * m);
```

Replaces the CTM with `m`.

### cg_identity_matrix

```c
void cg_identity_matrix(struct cg_ctx_t * ctx);
```

Resets the CTM to the identity matrix.

## Path Construction

These functions build the current path. Coordinates are in user space and will be transformed by the CTM at render time.

### Basic Operations

#### cg_move_to

```c
void cg_move_to(struct cg_ctx_t * ctx, float x, float y);
```

Begins a new sub-path at `(x, y)`.

#### cg_line_to

```c
void cg_line_to(struct cg_ctx_t * ctx, float x, float y);
```

Adds a line segment to `(x, y)`.

#### cg_quad_to

```c
void cg_quad_to(struct cg_ctx_t * ctx, float x1, float y1, float x2, float y2);
```

Adds a quadratic Bezier curve with control point `(x1, y1)` and end point `(x2, y2)`. Internally converted to cubic.

#### cg_cubic_to

```c
void cg_cubic_to(struct cg_ctx_t * ctx,
    float x1, float y1,
    float x2, float y2,
    float x3, float y3);
```

Adds a cubic Bezier curve.

#### cg_arc_to

```c
void cg_arc_to(struct cg_ctx_t * ctx,
    float rx, float ry, float angle,
    int large, int sweep,
    float x, float y);
```

Adds an elliptical arc. See [cg_path_arc_to](./path#cg_path_arc_to) for details.

### Relative Operations

These functions compute the target coordinates by adding deltas to the current point:

```c
void cg_rel_move_to(struct cg_ctx_t * ctx, float dx, float dy);
void cg_rel_line_to(struct cg_ctx_t * ctx, float dx, float dy);
void cg_rel_quad_to(struct cg_ctx_t * ctx, float dx1, float dy1, float dx2, float dy2);
void cg_rel_cubic_to(struct cg_ctx_t * ctx, float dx1, float dy1, float dx2, float dy2, float dx3, float dy3);
void cg_rel_arc_to(struct cg_ctx_t * ctx, float rx, float ry, float angle, int large, int sweep, float dx, float dy);
```

For example, `cg_rel_line_to(ctx, 10, 0)` draws a line 10 units to the right from the current point.

### Predefined Shapes

Each of these functions adds a complete closed sub-path:

```c
void cg_rectangle(struct cg_ctx_t * ctx, float x, float y, float w, float h);
void cg_round_rectangle(struct cg_ctx_t * ctx, float x, float y, float w, float h, float rx, float ry);
void cg_ellipse(struct cg_ctx_t * ctx, float cx, float cy, float rx, float ry);
void cg_circle(struct cg_ctx_t * ctx, float cx, float cy, float r);
void cg_arc(struct cg_ctx_t * ctx, float cx, float cy, float r, float a0, float a1);
void cg_arc_negative(struct cg_ctx_t * ctx, float cx, float cy, float r, float a0, float a1);
```

`cg_arc` draws in the positive (clockwise) direction; `cg_arc_negative` draws in the counter-clockwise direction.

### Path Management

#### cg_new_path

```c
void cg_new_path(struct cg_ctx_t * ctx);
```

Clears all segments, resetting the path to empty.

#### cg_new_sub_path

```c
void cg_new_sub_path(struct cg_ctx_t * ctx);
```

Begins a new sub-path. The next drawing command will start at the given point.

#### cg_close_path

```c
void cg_close_path(struct cg_ctx_t * ctx);
```

Closes the current sub-path with a straight line back to its starting point.

#### cg_add_path

```c
void cg_add_path(struct cg_ctx_t * ctx, struct cg_path_t * path);
```

Appends an external path to the current path. The path elements are copied directly (no transformation).

## Rendering

### cg_clip

```c
void cg_clip(struct cg_ctx_t * ctx);
```

Rasterizes the current path using the current fill rule and sets it as the clip region, replacing any previous clip region. Then clears the path. Subsequent drawing is visible only within the clip region.

:::note
The clip region is stored as a span buffer. On the first clip call, the path is rasterized and stored. Subsequent clip calls intersect with the existing clip spans.
:::

### cg_clip_preserve

```c
void cg_clip_preserve(struct cg_ctx_t * ctx);
```

Same as `cg_clip`, but does not clear the path (useful for fill+stroke after clipping).

### cg_reset_clip

```c
void cg_reset_clip(struct cg_ctx_t * ctx);
```

Resets the clip region to the entire surface. Clears all accumulated clip spans and disables clipping.

### cg_fill

```c
void cg_fill(struct cg_ctx_t * ctx);
```

Rasterizes the current path using the current fill rule, then blends the paint onto the surface within the filled region (intersected with the active clip region). Clears the path afterward.

### cg_fill_preserve

```c
void cg_fill_preserve(struct cg_ctx_t * ctx);
```

Same as `cg_fill`, but preserves the path for subsequent operations (e.g., fill then stroke).

### cg_mask

```c
void cg_mask(struct cg_ctx_t * ctx, struct cg_paint_t * paint);
```

A drawing operator that paints the current source using the alpha channel of `paint` as a per-pixel mask within the filled area of the current path. Opaque areas of the mask paint are painted with the source; transparent areas are not painted. Clears the path after completion.

### cg_mask_surface

```c
void cg_mask_surface(struct cg_ctx_t * ctx, struct cg_surface_t * mask, float x, float y);
```

A convenience wrapper that wraps `mask` into a texture paint (positioned at user-space `(x, y)`) and then calls `cg_mask(ctx, paint)`.

### cg_stroke

```c
void cg_stroke(struct cg_ctx_t * ctx);
```

Strokes the current path using the current stroke style (line width, cap, join, miter limit, dash pattern). If dashes are set, the path is dashed first, then stroked using the underlying FreeType stroker. Clears the path.

### cg_stroke_preserve

```c
void cg_stroke_preserve(struct cg_ctx_t * ctx);
```

Same as `cg_stroke`, but preserves the path.

### cg_paint

```c
void cg_paint(struct cg_ctx_t * ctx);
```

Fills the entire clip region (or the entire surface if no clip is set) with the current paint. Does not use the path. Typically used to cover an entire area with a texture or gradient.

### cg_paint_with_alpha

```c
void cg_paint_with_alpha(struct cg_ctx_t * ctx, float alpha);
```

Fills the entire clip region with the current paint using the specified alpha value. Equivalent to `cg_save(ctx)` + `cg_set_opacity(ctx, alpha)` + `cg_paint(ctx)` + `cg_restore(ctx)`. The original opacity setting is preserved after the call.
