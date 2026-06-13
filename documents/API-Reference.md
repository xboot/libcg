# libcg API Reference

libcg is a tiny C library for 2D computer graphics, inspired by the Cairo / HTML Canvas drawing model. It renders anti-aliased graphics into an RGBA32 pixel buffer via span-based rasterization.

---

## Table of Contents

1. [Architecture Overview](#architecture-overview)
2. [Data Types](#data-types)
    - [Basic Structures](#basic-structures)
    - [Enumerations](#enumerations)
    - [Object Structures](#object-structures)
3. [Matrix Functions](#matrix-functions)
4. [Surface Functions](#surface-functions)
5. [Paint Functions](#paint-functions)
6. [Path Functions](#path-functions)
7. [Context Functions](#context-functions)
    - [Creation and Destruction](#creation-and-destruction)
    - [State Management](#state-management)
    - [Query Functions](#query-functions)
    - [Spatial Queries](#spatial-queries)
    - [Paint and Style](#paint-and-style)
    - [Transformations](#transformations)
    - [Path Construction](#path-construction)
    - [Rendering](#rendering)
8. [Complete Examples](#complete-examples)

---

## Architecture Overview

libcg follows a stateful drawing model. The central object is `cg_ctx_t` (the drawing context), which holds:

- A reference to the target `cg_surface_t` pixel buffer
- The current `cg_path_t` being constructed
- A stack of `cg_state_t` objects (push/pop via `cg_save`/`cg_restore`)

Each state stores: current paint (solid, gradient, or texture), transformation matrix, stroke style (width, cap, join, miter limit, dash pattern), fill rule, compositing operator, global opacity, and the clip region.

**Memory management**: Surface, Paint, and Path objects use reference counting. Functions that accept these objects will increment the reference count as needed (e.g., `cg_set_paint` / `cg_paint_create_texture` call `cg_paint_reference` and `cg_surface_reference` internally). You must pair every `_create` with the corresponding `_destroy`.

**Default state values** (when a context is created):

| Property     | Default Value                |
|-------------|------------------------------|
| Paint       | `NULL` (interprets as `state->color`) |
| Color       | `{0, 0, 0, 1}` (black)      |
| Matrix      | Identity                     |
| Line width  | `1.0`                        |
| Line cap    | `CG_LINE_CAP_BUTT`           |
| Line join   | `CG_LINE_JOIN_MITER`         |
| Miter limit | `10.0`                       |
| Fill rule   | `CG_FILL_RULE_NON_ZERO`      |
| Operator    | `CG_OPERATOR_SRC_OVER`       |
| Opacity     | `1.0`                        |

**Color component values** throughout the API are floats in `[0.0, 1.0]`. The library clamps values to this range internally.

**Coordinate system**: Origin at top-left, X increases to the right, Y increases downward. Angles are in radians, positive direction is clockwise.

---

## Data Types

### Basic Structures

#### cg_point_t

```c
struct cg_point_t {
    float x;
    float y;
};
```

A 2D point.

#### cg_rect_t

```c
struct cg_rect_t {
    float x;
    float y;
    float w;
    float h;
};
```

An axis-aligned rectangle anchored at the top-left corner `(x, y)` with width `w` and height `h`.

#### cg_matrix_t

```c
struct cg_matrix_t {
    float a;  float b;
    float c;  float d;
    float tx; float ty;
};
```

A 2D affine transformation matrix mapping a point `(x, y)` to:

```
x' = a*x + c*y + tx
y' = b*x + d*y + ty
```

In homogeneous form:
```
| a  c  tx |
| b  d  ty |
| 0  0  1  |
```

#### cg_color_t

```c
struct cg_color_t {
    float r;
    float g;
    float b;
    float a;
};
```

RGBA color. All components clamped to `[0.0, 1.0]` when constructing paints.

#### cg_gradient_stop_t

```c
struct cg_gradient_stop_t {
    float offset;
    struct cg_color_t color;
};
```

Defines a position-color pair in a gradient. `offset` must be in `[0.0, 1.0]`. Offsets are clamped and sorted internally — if a stop has an offset smaller than the previous stop, it is bumped up to that value.

---

### Enumerations

#### cg_path_command_t

Used internally to describe path elements:

| Value                       | Points | Description                    |
|-----------------------------|--------|--------------------------------|
| `CG_PATH_COMMAND_MOVE_TO`   | 1      | Moves to a new point, starts a sub-path |
| `CG_PATH_COMMAND_LINE_TO`   | 1      | Line segment to a point        |
| `CG_PATH_COMMAND_CUBIC_TO`  | 3      | Cubic Bezier curve (control point 1, control point 2, end point) |
| `CG_PATH_COMMAND_CLOSE`     | 1      | Closes the current sub-path    |

*Note: Quadratic Bezier curves are internally converted to cubic when added to a path.*

#### cg_spread_method_t

Controls how gradient colors are sampled outside the `[0, 1]` range:

| Value                       | Description                                      |
|-----------------------------|--------------------------------------------------|
| `CG_SPREAD_METHOD_PAD`      | Clamp to the nearest edge color                  |
| `CG_SPREAD_METHOD_REFLECT`  | Mirror the gradient at each integer boundary     |
| `CG_SPREAD_METHOD_REPEAT`   | Repeat the gradient (wrap around)                |

#### cg_gradient_type_t

| Value                     | Description     |
|---------------------------|-----------------|
| `CG_GRADIENT_TYPE_LINEAR` | Linear gradient |
| `CG_GRADIENT_TYPE_RADIAL` | Radial gradient |

#### cg_texture_type_t

| Value                   | Description                                                       |
|-------------------------|-------------------------------------------------------------------|
| `CG_TEXTURE_TYPE_PLAIN` | Single copy of the texture drawn once (regions outside are transparent) |
| `CG_TEXTURE_TYPE_TILED` | Texture repeats to fill the entire painted area                   |

*Note: Transformed non-tiled textures, and tiled textures with rotation/shear, use bilinear filtering. Other combinations use nearest-neighbor sampling.*

#### cg_line_cap_t

| Value                 | Description                     |
|-----------------------|--------------------------------|
| `CG_LINE_CAP_BUTT`   | Flat end, exactly at the endpoint (default) |
| `CG_LINE_CAP_ROUND`  | Semicircular end extending beyond the endpoint |
| `CG_LINE_CAP_SQUARE` | Square end extending half the line width beyond the endpoint |

#### cg_line_join_t

| Value                  | Description                                          |
|------------------------|------------------------------------------------------|
| `CG_LINE_JOIN_MITER`  | Sharp corner, clipped by miter limit (default)       |
| `CG_LINE_JOIN_ROUND`  | Rounded corner                                       |
| `CG_LINE_JOIN_BEVEL`  | Flat (beveled) corner                                |

#### cg_fill_rule_t

Determines which regions are "inside" a self-intersecting path:

| Value                       | Description                        |
|-----------------------------|------------------------------------|
| `CG_FILL_RULE_NON_ZERO`     | Non-zero winding rule (default)    |
| `CG_FILL_RULE_EVEN_ODD`     | Even-odd rule                      |

#### cg_operator_t

Porter-Duff compositing operators. These control how new pixels are blended with existing content on the surface:

| Value                     | Formula (short)                      | Effect                                      |
|---------------------------|--------------------------------------|---------------------------------------------|
| `CG_OPERATOR_CLEAR`       | 0                                     | Clear destination                           |
| `CG_OPERATOR_SRC`         | S                                     | Copy source                                 |
| `CG_OPERATOR_DST`         | D                                     | Leave destination unchanged*                |
| `CG_OPERATOR_SRC_OVER`    | S + D*(1 - Aₛ)                        | Source on top (default)                     |
| `CG_OPERATOR_DST_OVER`    | D + S*(1 - Aᴅ)                        | Destination on top                          |
| `CG_OPERATOR_SRC_IN`      | S * Aᴅ                                | Source where destination exists             |
| `CG_OPERATOR_DST_IN`      | D * Aₛ                                | Destination where source exists             |
| `CG_OPERATOR_SRC_OUT`     | S * (1 - Aᴅ)                          | Source where destination is absent          |
| `CG_OPERATOR_DST_OUT`     | D * (1 - Aₛ)                          | Destination where source is absent          |
| `CG_OPERATOR_SRC_ATOP`    | S*Aᴅ + D*(1 - Aₛ)                     | Source on top of destination, trimmed       |
| `CG_OPERATOR_DST_ATOP`    | D*Aₛ + S*(1 - Aᴅ)                     | Destination on top of source, trimmed       |
| `CG_OPERATOR_XOR`         | S*(1 - Aᴅ) + D*(1 - Aₛ)              | Non-overlapping regions combined            |

*Where S=source, D=destination, Aₛ=source alpha, Aᴅ=destination alpha*

*When the paint is a solid color with alpha=255 and operator is SRC_OVER, the library optimizes by using the SRC operator internally.*

#### cg_paint_type_t

| Value                      | Description     |
|----------------------------|-----------------|
| `CG_PAINT_TYPE_COLOR`      | Solid color     |
| `CG_PAINT_TYPE_GRADIENT`   | Gradient        |
| `CG_PAINT_TYPE_TEXTURE`    | Texture/image   |

---

### Object Structures

#### cg_surface_t

```c
struct cg_surface_t {
    int refcnt;
    int width;
    int height;
    int stride;
    int owndata;
    unsigned char * pixels;
};
```

The pixel canvas. Pixels are stored in **premultiplied RGBA32** format (row-major). `stride` equals `width * 4`. `owndata` is `1` when the surface allocated its own buffer (via `cg_surface_create`) and `0` when wrapping external data (via `cg_surface_create_for_data`).

#### cg_path_t

```c
struct cg_path_t {
    int refcnt;
    int num_points;
    int num_contours;
    int num_curves;
    int sub_path;
    struct cg_point_t start_point;
    struct { union cg_path_element_t * data; int size; int capacity; } elements;
};
```

A vector path composed of sub-paths (contours). Each sub-path begins with a `MOVE_TO` and optionally ends with `CLOSE`. Quadratic Beziers are stored as cubic. Quadratic-to-cubic conversion uses:

```
CP1 = 2/3 * Q1 + 1/3 * P0
CP2 = 2/3 * Q1 + 1/3 * P2
```

#### cg_paint_t / cg_solid_paint_t / cg_gradient_paint_t / cg_texture_paint_t

Paint is the "what to draw" — it can be a solid color, gradient, or texture. The `cg_paint_t` struct serves as a base with a `type` discriminator. Access the concrete type through the appropriate subtype pointer.

**Gradient paint** stores color stops inline (allocated in the same block as the struct). `values[6]` stores geometry:
- Linear: `{x1, y1, x2, y2}` 
- Radial: `{cx1, cy1, r1, cx0, cy0, r0}` (end circle, then start circle)

**Texture paint** references a surface and stores opacity, texture type, and an affine matrix.

---

## Matrix Functions

Matrices transform coordinates from user space to surface space. All matrix functions operate in-place on the matrix pointed to by `m`.

### cg_matrix_init

```c
void cg_matrix_init(struct cg_matrix_t * m, float a, float b, float c, float d, float tx, float ty);
```

Initialize a matrix with explicit values.

### cg_matrix_init_identity

```c
void cg_matrix_init_identity(struct cg_matrix_t * m);
```

Set matrix to identity: `{1, 0, 0, 1, 0, 0}`.

### cg_matrix_init_translate

```c
void cg_matrix_init_translate(struct cg_matrix_t * m, float tx, float ty);
```

Initialize as a translation-only matrix: `{1, 0, 0, 1, tx, ty}`.

### cg_matrix_init_scale

```c
void cg_matrix_init_scale(struct cg_matrix_t * m, float sx, float sy);
```

Initialize as a scale-only matrix: `{sx, 0, 0, sy, 0, 0}`.

### cg_matrix_init_rotate

```c
void cg_matrix_init_rotate(struct cg_matrix_t * m, float r);
```

Initialize as a rotation matrix by `r` radians: `{cos(r), sin(r), -sin(r), cos(r), 0, 0}`.

### cg_matrix_init_shear

```c
void cg_matrix_init_shear(struct cg_matrix_t * m, float shx, float shy);
```

Initialize as a shear matrix: `{1, shy, shx, 1, 0, 0}`.

### cg_matrix_translate

```c
void cg_matrix_translate(struct cg_matrix_t * m, float tx, float ty);
```

Post-multiply a translation onto the existing matrix:
```
tx' = tx + a*dx + c*dy
ty' = ty + b*dx + d*dy
```

### cg_matrix_scale

```c
void cg_matrix_scale(struct cg_matrix_t * m, float sx, float sy);
```

Post-multiply a scale: `a*=sx; b*=sx; c*=sy; d*=sy`.

### cg_matrix_rotate

```c
void cg_matrix_rotate(struct cg_matrix_t * m, float r);
```

Post-multiply a rotation by `r` radians.

### cg_matrix_shear

```c
void cg_matrix_shear(struct cg_matrix_t * m, float shx, float shy);
```

Post-multiply a shear: `m = m * shear(shx, shy)`.

### cg_matrix_multiply

```c
void cg_matrix_multiply(struct cg_matrix_t * m, struct cg_matrix_t * m1, struct cg_matrix_t * m2);
```

Set `m = m1 * m2`. An optimization path is used when both matrices are axis-aligned (i.e., `b == 0` and `c == 0` for both).

### cg_matrix_invert

```c
int cg_matrix_invert(struct cg_matrix_t * m);
```

Invert the matrix in place. Returns non-zero on success, `0` if the matrix is singular.

Special cases:
- If `b == 0` and `c == 0` (axis-aligned): returns `0` if `a == 0` or `d == 0`.
- Otherwise: returns `0` if `|det| <= 1e-20`.

---

## Surface Functions

### cg_surface_create

```c
struct cg_surface_t * cg_surface_create(int width, int height);
```

Allocates a new surface of the given size. The pixel buffer is zero-initialized (fully transparent black). `owndata` is set to `1`.

### cg_surface_create_for_data

```c
struct cg_surface_t * cg_surface_create_for_data(int width, int height, void * pixels);
```

Creates a surface wrapping an existing pixel buffer. `pixels` must be at least `width * height * 4` bytes in premultiplied RGBA32 format. `owndata` is set to `0`, meaning calling `cg_surface_destroy` will **not** free the pixel data — the caller is responsible for its lifetime.

### cg_surface_destroy

```c
void cg_surface_destroy(struct cg_surface_t * surface);
```

Decrements the reference count. When it reaches zero, frees the pixel buffer (if `owndata == 1`) and the surface struct. Safe to call with `NULL`.

### cg_surface_reference

```c
struct cg_surface_t * cg_surface_reference(struct cg_surface_t * surface);
```

Increments the reference count and returns the surface. Safe to call with `NULL` (returns `NULL`).

---

## Paint Functions

Paint objects are standalone — they can be created once and reused across multiple drawing operations or contexts. Use `cg_paint_destroy` when done.

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
| `stops`   | Array of `nstops` color stops (offsets clamped & sorted)        |
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

*Note: The `values` array in the paint is stored as `{cx1, cy1, r1, cx0, cy0, r0}` (end circle first, then start circle).*

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

### cg_paint_destroy

```c
void cg_paint_destroy(struct cg_paint_t * paint);
```

Decrements reference count. When zero:
- For texture paints: calls `cg_surface_destroy` on the referenced surface.
- For all paints: frees the memory.

### cg_paint_reference

```c
struct cg_paint_t * cg_paint_reference(struct cg_paint_t * paint);
```

Increments reference count and returns the paint. Safe with `NULL`.

---

## Path Functions

Path objects can be built independently of a context and reused.

### Lifecycle

#### cg_path_create

```c
struct cg_path_t * cg_path_create(void);
```

Creates an empty path with no sub-paths.

#### cg_path_destroy

```c
void cg_path_destroy(struct cg_path_t * path);
```

Decrements reference count, frees path data when zero.

#### cg_path_reference

```c
struct cg_path_t * cg_path_reference(struct cg_path_t * path);
```

Increments reference count, returns the path.

### Path Manipulation

#### cg_path_reset

```c
void cg_path_reset(struct cg_path_t * path);
```

Clears all elements, contour data, and curve count. Resets to an empty path.

#### cg_path_new_sub_path

```c
void cg_path_new_sub_path(struct cg_path_t * path);
```

Marks that the next drawing command should begin a new sub-path. If the next command is `line_to`, `cubic_to`, etc. without a preceding `move_to`, it will be promoted to a `move_to`.

#### cg_path_close

```c
void cg_path_close(struct cg_path_t * path);
```

Closes the current sub-path by adding a `CLOSE` command referencing the sub-path's start point. Does nothing if the path is empty. Resets the `sub_path` flag.

#### cg_path_transform

```c
void cg_path_transform(struct cg_path_t * path, struct cg_matrix_t * m);
```

Transforms all points in the path by matrix `m` in place. Traverses all move, line, close (1 point) and cubic (3 points) commands and maps each point through the matrix.

#### cg_path_get_current_point

```c
void cg_path_get_current_point(struct cg_path_t * path, float * x, float * y);
```

Returns the current position. If there are no points or the path is in "new sub-path" mode, returns `(0, 0)`. Either `x` or `y` may be `NULL` if only one coordinate is needed.

### Building Paths

Paths are built incrementally. The first drawing command implicitly creates a `MOVE_TO` to `(0, 0)` if the path is empty.

#### cg_path_move_to

```c
void cg_path_move_to(struct cg_path_t * path, float x, float y);
```

Starts a new sub-path at `(x, y)`. Increments the contour count. Resets the `sub_path` flag.

#### cg_path_line_to

```c
void cg_path_line_to(struct cg_path_t * path, float x, float y);
```

Adds a straight line segment from the current point to `(x, y)`.

- If `sub_path` is set, promotes to `move_to(x, y)`.
- If the path is empty, first adds `move_to(0, 0)`.

#### cg_path_quad_to

```c
void cg_path_quad_to(struct cg_path_t * path, float x1, float y1, float x2, float y2);
```

Adds a quadratic Bezier curve. Internally converted to cubic:
```
CP1 = 2/3 * (x1, y1) + 1/3 * current_point
CP2 = 2/3 * (x1, y1) + 1/3 * (x2, y2)
```

If the path is empty, first adds `move_to(0, 0)`.

#### cg_path_cubic_to

```c
void cg_path_cubic_to(struct cg_path_t * path,
    float x1, float y1,
    float x2, float y2,
    float x3, float y3);
```

Adds a cubic Bezier curve with two control points `(x1, y1)`, `(x2, y2)` and end point `(x3, y3)`. Increments the curve count.

If the path is empty or `sub_path` is set, first adds `move_to(0, 0)`.

#### cg_path_arc_to

```c
void cg_path_arc_to(struct cg_path_t * path,
    float rx, float ry,
    float angle,
    int large,
    int sweep,
    float x, float y);
```

Adds an elliptical arc from the current point to `(x, y)`.

| Parameter | Description                                                            |
|-----------|------------------------------------------------------------------------|
| `rx, ry`  | Ellipse radii (negatives are treated as absolute values)               |
| `angle`   | Rotation of the ellipse in radians                                     |
| `large`   | Non-zero to take the larger arc (arc > 180°)                           |
| `sweep`   | Non-zero for positive (clockwise) angle direction                      |
| `x, y`    | End point of the arc                                                   |

Edge cases:
- If `rx == 0` or `ry == 0`, or if start equals end, falls back to `cg_path_line_to`.
- If the radii are too small to reach the endpoint, they are scaled up proportionally.

### Predefined Shapes

#### cg_path_add_rectangle

```c
void cg_path_add_rectangle(struct cg_path_t * path, float x, float y, float w, float h);
```

Adds a closed rectangle as a new sub-path: `move_to(x, y)` → `line_to(x+w, y)` → `line_to(x+w, y+h)` → `line_to(x, y+h)` → `line_to(x, y)` → `close`.

#### cg_path_add_round_rectangle

```c
void cg_path_add_round_rectangle(struct cg_path_t * path,
    float x, float y,
    float w, float h,
    float rx, float ry);
```

Adds a closed rounded rectangle. Corner radii are clamped to `min(w/2, h/2)`. If both radii are zero, falls back to `cg_path_add_rectangle`. Uses cubic Beziers with control points at `radius * 0.5522847498` (KAPPA) to approximate quarter-circles.

#### cg_path_add_ellipse

```c
void cg_path_add_ellipse(struct cg_path_t * path, float cx, float cy, float rx, float ry);
```

Adds a closed ellipse centered at `(cx, cy)` with radii `(rx, ry)`. Uses 4 cubic Bezier curves per ellipse.

#### cg_path_add_circle

```c
void cg_path_add_circle(struct cg_path_t * path, float cx, float cy, float r);
```

Convenience wrapper: calls `cg_path_add_ellipse(path, cx, cy, r, r)`.

#### cg_path_add_arc

```c
void cg_path_add_arc(struct cg_path_t * path,
    float cx, float cy, float r,
    float a0, float a1,
    int ccw);
```

Adds a circular arc centered at `(cx, cy)` from angle `a0` to `a1`.

| Parameter | Description                                                           |
|-----------|-----------------------------------------------------------------------|
| `a0`      | Start angle in radians                                                |
| `a1`      | End angle in radians                                                  |
| `ccw`     | Non-zero for counter-clockwise direction                              |

Behavior:
- If `|a1 - a0| > 2π`, the difference is clamped to `2π`.
- If direction mismatch (e.g., `a0 < a1` but `ccw` is set), adds or subtracts `2π`.
- If the path is empty or in `sub_path` mode, begins with a `move_to` to the start point; otherwise adds a `line_to`.
- Each `π/2` segment is subdivided into one cubic Bezier.

#### cg_path_add_path

```c
void cg_path_add_path(struct cg_path_t * path,
    struct cg_path_t * source,
    struct cg_matrix_t * m);
```

Appends `source` to the current path. If `m` is `NULL`, the source elements are copied directly. If `m` is non-`NULL`, each point in `source` is transformed by `m` before being added.

#### cg_path_extents

```c
float cg_path_extents(struct cg_path_t * path, struct cg_rect_t * extents, int tight);
```

Computes the bounding box of the path.

| Parameter | Description                                                              |
|-----------|--------------------------------------------------------------------------|
| `extents` | Output rectangle, or `NULL` if not needed                                |
| `tight`   | Non-zero: flattens Bezier curves for exact extents. 0: uses control points only |

**Returns**: The approximate arc length of the path (sum of segment lengths), not the area.

---

## Context Functions

### Creation and Destruction

#### cg_create

```c
struct cg_ctx_t * cg_create(struct cg_surface_t * surface);
```

Creates a drawing context bound to `surface`. The surface is referenced. Initializes a default state, an empty path, and sets the clip rectangle to cover the entire surface `{0, 0, width, height}`.

#### cg_destroy

```c
void cg_destroy(struct cg_ctx_t * ctx);
```

Destroys the context. Frees all saved states, the freed-state pool, span buffers, the surface reference, and the path. Safe with `NULL`.

### State Management

Drawing states are stored on a linked-list stack. `cg_save` pushes a copy of the current state; `cg_restore` pops it. States include: paint, color, matrix, stroke style (width/cap/join/miter/dash), fill rule, operator, opacity, and clip region.

The library maintains a freed-state pool to avoid repeated `malloc`/`free` during save/restore cycles.

#### cg_save

```c
void cg_save(struct cg_ctx_t * ctx);
```

Pushes the current state onto the stack. All subsequent modifications to paint, matrix, stroke style, etc. are isolated until `cg_restore` is called.

#### cg_restore

```c
void cg_restore(struct cg_ctx_t * ctx);
```

Pops the top state from the stack and restores it. Does nothing if only one state remains (the initial state is never popped).

### Query Functions

#### cg_get_paint

```c
struct cg_paint_t * cg_get_paint(struct cg_ctx_t * ctx, struct cg_color_t * color);
```

Returns the current paint object (may be `NULL` if using simple color mode). If `color` is non-`NULL` and a paint exists, the color is filled from the state's stored color (which may differ from the paint's color — the state snapshot color tracks the last `cg_set_source_*` value).

*Note: When you call `cg_set_source_rgba`, it sets `state->color` to the given RGBA and sets `state->paint = NULL`. So `cg_get_paint` would return `NULL` but `cg_get_paint(ctx, &c)` would fill `c` with the correct color.*

#### cg_get_surface

```c
struct cg_surface_t * cg_get_surface(struct cg_ctx_t * ctx);
```

Returns the surface bound to this context.

#### cg_get_path

```c
struct cg_path_t * cg_get_path(struct cg_ctx_t * ctx);
```

Returns the current path being constructed.

#### cg_get_matrix

```c
struct cg_matrix_t * cg_get_matrix(struct cg_ctx_t * ctx);
```

Returns a pointer to the current transformation matrix (mutable — changes affect the context).

#### cg_get_operator

```c
enum cg_operator_t cg_get_operator(struct cg_ctx_t * ctx);
```

Returns the current compositing operator.

#### cg_get_opacity

```c
float cg_get_opacity(struct cg_ctx_t * ctx);
```

Returns the current global opacity.

#### cg_get_current_point

```c
void cg_get_current_point(struct cg_ctx_t * ctx, float * x, float * y);
```

Returns the current path position.

### Spatial Queries

These functions rasterize the current path to determine spatial relationships. They are potentially expensive if called repeatedly in a loop.

#### cg_fill_extents

```c
void cg_fill_extents(struct cg_ctx_t * ctx, struct cg_rect_t * extents);
```

Rasterizes the current path with the current fill rule and writes the bounding box to `extents`.

#### cg_stroke_extents

```c
void cg_stroke_extents(struct cg_ctx_t * ctx, struct cg_rect_t * extents);
```

Rasterizes the current path with the current stroke style (width, cap, join, dashes) and writes the bounding box to `extents`.

#### cg_clip_extents

```c
void cg_clip_extents(struct cg_ctx_t * ctx, struct cg_rect_t * extents);
```

Returns the bounding box of the current clip region. If clipping is active, returns the clip span extents. Otherwise, returns the initial clip rectangle.

#### cg_in_fill

```c
int cg_in_fill(struct cg_ctx_t * ctx, float x, float y);
```

Tests if the point `(x, y)` is inside the filled area of the current path.

#### cg_in_stroke

```c
int cg_in_stroke(struct cg_ctx_t * ctx, float x, float y);
```

Tests if the point `(x, y)` is inside the stroked area of the current path.

#### cg_in_clip

```c
int cg_in_clip(struct cg_ctx_t * ctx, float x, float y);
```

Tests if the point `(x, y)` is inside the current clip region.

### Paint and Style

#### cg_set_paint

```c
void cg_set_paint(struct cg_ctx_t * ctx, struct cg_paint_t * paint);
```

Sets the current paint. The paint is referenced. Passing `NULL` reverts to using the state's stored color.

#### cg_set_source_rgb

```c
void cg_set_source_rgb(struct cg_ctx_t * ctx, float r, float g, float b);
```

Sets the paint to a solid RGB color (alpha = 1.0). Internally stores the color in the state and sets paint to `NULL`.

#### cg_set_source_rgba

```c
void cg_set_source_rgba(struct cg_ctx_t * ctx, float r, float g, float b, float a);
```

Sets the paint to a solid RGBA color.

#### cg_set_source_color

```c
void cg_set_source_color(struct cg_ctx_t * ctx, struct cg_color_t * color);
```

Sets the paint to a solid color from a `cg_color_t` struct.

#### cg_set_source_surface

```c
void cg_set_source_surface(struct cg_ctx_t * ctx, struct cg_surface_t * surface, float x, float y);
```

Creates a plain (non-tiled) texture paint from `surface` with a translation offset of `(x, y)`. The surface is referenced. Internally equivalent to:

```c
cg_set_texture(ctx, surface, CG_TEXTURE_TYPE_PLAIN, 1.0, &translate_matrix);
```

#### cg_set_linear_gradient

```c
void cg_set_linear_gradient(struct cg_ctx_t * ctx,
    float x1, float y1, float x2, float y2,
    enum cg_spread_method_t spread,
    struct cg_gradient_stop_t * stops, int nstops,
    struct cg_matrix_t * m);
```

Creates and sets a linear gradient paint. See `cg_paint_create_linear_gradient` for parameter details.

#### cg_set_radial_gradient

```c
void cg_set_radial_gradient(struct cg_ctx_t * ctx,
    float cx0, float cy0, float r0,
    float cx1, float cy1, float r1,
    enum cg_spread_method_t spread,
    struct cg_gradient_stop_t * stops, int nstops,
    struct cg_matrix_t * m);
```

Creates and sets a radial gradient paint. See `cg_paint_create_radial_gradient` for parameter details.

#### cg_set_texture

```c
void cg_set_texture(struct cg_ctx_t * ctx,
    struct cg_surface_t * surface,
    enum cg_texture_type_t type,
    float opacity,
    struct cg_matrix_t * m);
```

Creates and sets a texture paint. See `cg_paint_create_texture` for parameter details.

#### cg_set_operator

```c
void cg_set_operator(struct cg_ctx_t * ctx, enum cg_operator_t op);
```

Sets the compositing operator used when blending onto the surface.

#### cg_set_opacity

```c
void cg_set_opacity(struct cg_ctx_t * ctx, float opacity);
```

Sets the global opacity. Clamped to `[0.0, 1.0]`. This is multiplied with the paint's opacity and per-span coverage.

#### cg_set_fill_rule

```c
void cg_set_fill_rule(struct cg_ctx_t * ctx, enum cg_fill_rule_t winding);
```

Sets the fill rule. Applies to both filling and clipping.

#### cg_set_line_width

```c
void cg_set_line_width(struct cg_ctx_t * ctx, float width);
```

Sets the stroke line width. The width is affected by the current transformation matrix (scaling the CTM will change the apparent stroke width).

#### cg_set_line_cap

```c
void cg_set_line_cap(struct cg_ctx_t * ctx, enum cg_line_cap_t cap);
```

Sets how the ends of open sub-paths are drawn.

#### cg_set_line_join

```c
void cg_set_line_join(struct cg_ctx_t * ctx, enum cg_line_join_t join);
```

Sets how corners between connected line segments are drawn.

#### cg_set_miter_limit

```c
void cg_set_miter_limit(struct cg_ctx_t * ctx, float limit);
```

Sets the miter limit. When joining lines with `CG_LINE_JOIN_MITER`, if the miter would extend beyond `limit * line_width / 2`, the join is converted to a bevel.

#### cg_set_dash

```c
void cg_set_dash(struct cg_ctx_t * ctx, float * dashes, int ndashes, float offset);
```

Sets the dash pattern. `dashes` is an array of alternating dash/gap lengths (in user space). If the array has an odd number of elements, it is effectively doubled. `offset` specifies where in the pattern to start. Both the array and offset can be set independently with `cg_set_dash_array` and `cg_set_dash_offset`.

To disable dashing, set `ndashes` to `0`.

#### cg_set_dash_array

```c
void cg_set_dash_array(struct cg_ctx_t * ctx, float * dashes, int ndashes);
```

Sets only the dash array, preserving the current offset.

#### cg_set_dash_offset

```c
void cg_set_dash_offset(struct cg_ctx_t * ctx, float offset);
```

Sets only the dash offset, preserving the current array.

### Transformations

All transform functions post-multiply the given transform onto the current transformation matrix (CTM). This means transforms are applied in the order they are called — the last transform happens first.

#### cg_translate

```c
void cg_translate(struct cg_ctx_t * ctx, float tx, float ty);
```

Translates the CTM by `(tx, ty)`.

#### cg_scale

```c
void cg_scale(struct cg_ctx_t * ctx, float sx, float sy);
```

Scales the CTM by `(sx, sy)`.

#### cg_shear

```c
void cg_shear(struct cg_ctx_t * ctx, float shx, float shy);
```

Shears the CTM.

#### cg_rotate

```c
void cg_rotate(struct cg_ctx_t * ctx, float angle);
```

Rotates the CTM by `angle` radians clockwise.

#### cg_transform

```c
void cg_transform(struct cg_ctx_t * ctx, struct cg_matrix_t * m);
```

Post-multiplies the CTM by `m`: `CTM = m * CTM`.

#### cg_set_matrix

```c
void cg_set_matrix(struct cg_ctx_t * ctx, struct cg_matrix_t * m);
```

Replaces the CTM with `m`.

#### cg_identity_matrix

```c
void cg_identity_matrix(struct cg_ctx_t * ctx);
```

Resets the CTM to the identity matrix.

### Path Construction

These functions build the current path. Coordinates are in user space and will be transformed by the CTM at render time.

#### Basic Operations

##### cg_move_to

```c
void cg_move_to(struct cg_ctx_t * ctx, float x, float y);
```

Begins a new sub-path at `(x, y)`.

##### cg_line_to

```c
void cg_line_to(struct cg_ctx_t * ctx, float x, float y);
```

Adds a line segment to `(x, y)`.

##### cg_quad_to

```c
void cg_quad_to(struct cg_ctx_t * ctx, float x1, float y1, float x2, float y2);
```

Adds a quadratic Bezier curve with control point `(x1, y1)` and end point `(x2, y2)`. Internally converted to cubic.

##### cg_cubic_to

```c
void cg_cubic_to(struct cg_ctx_t * ctx,
    float x1, float y1,
    float x2, float y2,
    float x3, float y3);
```

Adds a cubic Bezier curve.

##### cg_arc_to

```c
void cg_arc_to(struct cg_ctx_t * ctx,
    float rx, float ry, float angle,
    int large, int sweep,
    float x, float y);
```

Adds an elliptical arc. See `cg_path_arc_to` for details.

#### Relative Operations

These functions compute the target coordinates by adding deltas to the current point:

```c
void cg_rel_move_to(struct cg_ctx_t * ctx, float dx, float dy);
void cg_rel_line_to(struct cg_ctx_t * ctx, float dx, float dy);
void cg_rel_quad_to(struct cg_ctx_t * ctx, float dx1, float dy1, float dx2, float dy2);
void cg_rel_cubic_to(struct cg_ctx_t * ctx, float dx1, float dy1, float dx2, float dy2, float dx3, float dy3);
void cg_rel_arc_to(struct cg_ctx_t * ctx, float rx, float ry, float angle, int large, int sweep, float dx, float dy);
```

For example, `cg_rel_line_to(ctx, 10, 0)` draws a line 10 units to the right from the current point.

#### Predefined Shapes

Each of these adds a complete closed sub-path:

```c
void cg_rectangle(struct cg_ctx_t * ctx, float x, float y, float w, float h);
void cg_round_rectangle(struct cg_ctx_t * ctx, float x, float y, float w, float h, float rx, float ry);
void cg_ellipse(struct cg_ctx_t * ctx, float cx, float cy, float rx, float ry);
void cg_circle(struct cg_ctx_t * ctx, float cx, float cy, float r);
void cg_arc(struct cg_ctx_t * ctx, float cx, float cy, float r, float a0, float a1);
void cg_arc_negative(struct cg_ctx_t * ctx, float cx, float cy, float r, float a0, float a1);
```

`cg_arc` draws in the positive (clockwise) direction; `cg_arc_negative` draws counter-clockwise.

#### Path Management

##### cg_new_path

```c
void cg_new_path(struct cg_ctx_t * ctx);
```

Clears all segments, resetting the path to empty.

##### cg_new_sub_path

```c
void cg_new_sub_path(struct cg_ctx_t * ctx);
```

Begins a new sub-path. The next drawing command will start at the given point.

##### cg_close_path

```c
void cg_close_path(struct cg_ctx_t * ctx);
```

Closes the current sub-path with a straight line back to its starting point.

##### cg_add_path

```c
void cg_add_path(struct cg_ctx_t * ctx, struct cg_path_t * path);
```

Appends an external path to the current path. The path elements are copied directly (no transformation).

### Rendering

#### cg_clip

```c
void cg_clip(struct cg_ctx_t * ctx);
```

Rasterizes the current path using the current fill rule and sets it as the clip region, replacing any previous clip. Then clears the path. Subsequent drawing is visible only where it overlaps the clip.

*Internally: The clip region is stored as a span buffer. On the first clip call, the path is rasterized and stored. Subsequent clip calls intersect with the existing clip spans.*

#### cg_clip_preserve

```c
void cg_clip_preserve(struct cg_ctx_t * ctx);
```

Same as `cg_clip`, but does not clear the path (useful for fill+stroke after clipping).

#### cg_fill

```c
void cg_fill(struct cg_ctx_t * ctx);
```

Rasterizes the current path with the current fill rule, then blends the paint onto the surface within the filled region (intersected with any active clip). Then clears the path.

Rendering pipeline: path → FT outline → rasterize to spans → intersect with clip spans (if any) → blend with active operator/opacity.

#### cg_fill_preserve

```c
void cg_fill_preserve(struct cg_ctx_t * ctx);
```

Same as `cg_fill`, but preserves the path for subsequent operations (e.g., fill then stroke).

#### cg_stroke

```c
void cg_stroke(struct cg_ctx_t * ctx);
```

Strokes the current path using the current stroke style (width, cap, join, miter limit, dash pattern). If dashes are set, the path is dashed first, then stroked using the underlying FreeType stroker. Clears the path.

#### cg_stroke_preserve

```c
void cg_stroke_preserve(struct cg_ctx_t * ctx);
```

Same as `cg_stroke`, but preserves the path.

#### cg_paint

```c
void cg_paint(struct cg_ctx_t * ctx);
```

Fills the entire clip region (or the entire surface if no clip is set) with the current paint. Does not use the path. This is typically used to fill a surface with a texture or gradient covering the whole area.

---

## Complete Examples

### Basic Red Rectangle with Blue Outline

```c
#include <cg.h>

struct cg_surface_t * surface = cg_surface_create(400, 300);
struct cg_ctx_t * ctx = cg_create(surface);

cg_set_source_rgba(ctx, 1.0, 0.0, 0.0, 1.0);
cg_rectangle(ctx, 50, 50, 100, 80);
cg_fill(ctx);

cg_set_source_rgba(ctx, 0.0, 0.0, 1.0, 1.0);
cg_set_line_width(ctx, 3.0);
cg_rectangle(ctx, 50, 50, 100, 80);
cg_stroke(ctx);

cg_destroy(ctx);
cg_surface_destroy(surface);
```

### Using Clipping

```c
struct cg_surface_t * surface = cg_surface_create(256, 256);
struct cg_ctx_t * ctx = cg_create(surface);

cg_arc(ctx, 128, 128, 76.8, 0, 2 * M_PI);
cg_clip(ctx);

cg_rectangle(ctx, 0, 0, 256, 256);
cg_fill(ctx);

cg_set_source_rgb(ctx, 0, 1, 0);
cg_move_to(ctx, 0, 0);
cg_line_to(ctx, 256, 256);
cg_set_line_width(ctx, 10.0);
cg_stroke(ctx);

cg_destroy(ctx);
cg_surface_destroy(surface);
```

### Linear Gradient

```c
struct cg_surface_t * surface = cg_surface_create(256, 256);
struct cg_ctx_t * ctx = cg_create(surface);

struct cg_gradient_stop_t stops[] = {
    { 0.0, { 1, 1, 1, 1 } },
    { 1.0, { 0, 0, 0, 1 } },
};

cg_rectangle(ctx, 0, 0, 256, 256);
cg_set_linear_gradient(ctx, 0, 0, 0, 256,
    CG_SPREAD_METHOD_PAD, stops, 2, NULL);
cg_fill(ctx);

cg_destroy(ctx);
cg_surface_destroy(surface);
```

### Transformations with Save/Restore

```c
struct cg_surface_t * surface = cg_surface_create(256, 256);
struct cg_ctx_t * ctx = cg_create(surface);

cg_set_source_rgb(ctx, 1, 0, 0);

cg_save(ctx);
cg_translate(ctx, 64, 64);
cg_rotate(ctx, M_PI / 4);
cg_rectangle(ctx, -30, -20, 60, 40);
cg_fill(ctx);
cg_restore(ctx);

cg_save(ctx);
cg_translate(ctx, 192, 192);
cg_scale(ctx, 2.0, 1.0);
cg_rectangle(ctx, -30, -20, 60, 40);
cg_fill(ctx);
cg_restore(ctx);

cg_destroy(ctx);
cg_surface_destroy(surface);
```

### Stroke Styles: Caps, Joins, and Dashes

```c
struct cg_surface_t * surface = cg_surface_create(256, 256);
struct cg_ctx_t * ctx = cg_create(surface);

float dashes[] = { 50, 10, 10, 10 };
cg_set_dash(ctx, dashes, 4, -50);
cg_set_line_width(ctx, 10);
cg_set_line_cap(ctx, CG_LINE_CAP_ROUND);
cg_set_line_join(ctx, CG_LINE_JOIN_ROUND);

cg_move_to(ctx, 128, 25.6);
cg_line_to(ctx, 230.4, 230.4);
cg_rel_line_to(ctx, -102.4, 0);
cg_cubic_to(ctx, 51.2, 230.4, 51.2, 128, 128, 128);
cg_stroke(ctx);

cg_destroy(ctx);
cg_surface_destroy(surface);
```

### Compositing Operators

```c
cg_set_operator(ctx, CG_OPERATOR_SRC_OVER);
cg_set_source_rgba(ctx, 1.0, 0.0, 0.0, 0.7);
cg_arc(ctx, cx, cy, r, 0, 2 * M_PI);
cg_fill(ctx);
```

### Texture with Tiling and Transform

```c
struct cg_surface_t * img = cg_surface_create_for_data(128, 128, pixels);
struct cg_surface_t * surface = cg_surface_create(256, 256);
struct cg_ctx_t * ctx = cg_create(surface);

cg_translate(ctx, 128, 128);
cg_rotate(ctx, -45 * M_PI / 180);
cg_scale(ctx, 0.8, 0.8);
cg_translate(ctx, -64, -64);
cg_set_texture(ctx, img, CG_TEXTURE_TYPE_TILED, 1.0, NULL);
cg_paint(ctx);

cg_destroy(ctx);
cg_surface_destroy(surface);
cg_surface_destroy(img);
```
