# Data Types

This document lists the definitions of all basic structs, enumeration types, and object structs in libcg.

## Basic Structures

### cg_point_t

```c
struct cg_point_t {
    float x;
    float y;
};
```

A 2D point.

### cg_rect_t

```c
struct cg_rect_t {
    float x;
    float y;
    float w;
    float h;
};
```

An axis-aligned rectangle anchored at the top-left corner `(x, y)` with width `w` and height `h`.

### cg_matrix_t

```c
struct cg_matrix_t {
    float a;  float b;
    float c;  float d;
    float tx; float ty;
};
```

A 2D affine transformation matrix that maps a point `(x, y)` to:

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

### cg_color_t

```c
struct cg_color_t {
    float r;
    float g;
    float b;
    float a;
};
```

RGBA color. All components are clamped to `[0.0, 1.0]` when constructing paints.

### cg_gradient_stop_t

```c
struct cg_gradient_stop_t {
    float offset;
    struct cg_color_t color;
};
```

Defines a position-color pair in a gradient. `offset` must be in the `[0.0, 1.0]` range. Offsets are clamped and sorted internally — if a stop has an offset smaller than the previous stop, it is bumped up to that stop's value.

## Enumerations

### cg_path_command_t

Used internally to describe path elements:

| Value                       | Points | Description                    |
|-----------------------------|--------|--------------------------------|
| `CG_PATH_COMMAND_MOVE_TO`   | 1      | Moves to a new point, starts a sub-path |
| `CG_PATH_COMMAND_LINE_TO`   | 1      | Line segment to a point        |
| `CG_PATH_COMMAND_CUBIC_TO`  | 3      | Cubic Bezier curve (control point 1, control point 2, end point) |
| `CG_PATH_COMMAND_CLOSE`     | 1      | Closes the current sub-path    |

:::note
Quadratic Bezier curves are internally converted to cubic when added to a path.
:::

### cg_spread_method_t

Controls how gradient colors are sampled outside the `[0, 1]` range:

| Value                       | Description                                      |
|-----------------------------|--------------------------------------------------|
| `CG_SPREAD_METHOD_PAD`      | Clamp to the nearest edge color                  |
| `CG_SPREAD_METHOD_REFLECT`  | Mirror the gradient at each integer boundary     |
| `CG_SPREAD_METHOD_REPEAT`   | Repeat the gradient (wrap around)                |

### cg_gradient_type_t

| Value                     | Description     |
|---------------------------|-----------------|
| `CG_GRADIENT_TYPE_LINEAR` | Linear gradient |
| `CG_GRADIENT_TYPE_RADIAL` | Radial gradient |

### cg_texture_type_t

| Value                   | Description                                                       |
|-------------------------|-------------------------------------------------------------------|
| `CG_TEXTURE_TYPE_PLAIN` | Single copy of the texture drawn once (regions outside are transparent) |
| `CG_TEXTURE_TYPE_TILED` | Texture repeats to fill the entire painted area                   |

:::note
Transformed non-tiled textures, and tiled textures with rotation/shear, use bilinear filtering. Other combinations use nearest-neighbor sampling.
:::

### cg_line_cap_t

| Value                 | Description                     |
|-----------------------|--------------------------------|
| `CG_LINE_CAP_BUTT`   | Flat end, exactly at the endpoint (default) |
| `CG_LINE_CAP_ROUND`  | Semicircular end extending beyond the endpoint |
| `CG_LINE_CAP_SQUARE` | Square end extending half the line width beyond the endpoint |

### cg_line_join_t

| Value                  | Description                                          |
|------------------------|------------------------------------------------------|
| `CG_LINE_JOIN_MITER`  | Sharp corner, clipped by miter limit (default)       |
| `CG_LINE_JOIN_ROUND`  | Rounded corner                                       |
| `CG_LINE_JOIN_BEVEL`  | Flat (beveled) corner                                |

### cg_fill_rule_t

Determines which regions are "inside" a self-intersecting path:

| Value                       | Description                        |
|-----------------------------|------------------------------------|
| `CG_FILL_RULE_NON_ZERO`     | Non-zero winding rule (default)    |
| `CG_FILL_RULE_EVEN_ODD`     | Even-odd rule                      |

### cg_operator_t

Porter-Duff compositing operators. These control how new pixels are blended with existing content on the surface. See the [Compositing Operators Reference](../reference/operators) for details.

| Value                     | Effect                                      |
|---------------------------|---------------------------------------------|
| `CG_OPERATOR_CLEAR`       | Clear destination                           |
| `CG_OPERATOR_SRC`         | Copy source                                 |
| `CG_OPERATOR_DST`         | Leave destination unchanged                 |
| `CG_OPERATOR_SRC_OVER`    | Source on top (default)                     |
| `CG_OPERATOR_DST_OVER`    | Destination on top                          |
| `CG_OPERATOR_SRC_IN`      | Source where destination exists             |
| `CG_OPERATOR_DST_IN`      | Destination where source exists             |
| `CG_OPERATOR_SRC_OUT`     | Source where destination is absent          |
| `CG_OPERATOR_DST_OUT`     | Destination where source is absent          |
| `CG_OPERATOR_SRC_ATOP`    | Source on top of destination, trimmed       |
| `CG_OPERATOR_DST_ATOP`    | Destination on top of source, trimmed       |
| `CG_OPERATOR_XOR`         | Non-overlapping regions combined            |

:::note
When the paint is a solid color with alpha=255 and the operator is SRC_OVER, the library optimizes by using the SRC operator internally.
:::

### cg_paint_type_t

| Value                      | Description     |
|----------------------------|-----------------|
| `CG_PAINT_TYPE_COLOR`      | Solid color     |
| `CG_PAINT_TYPE_GRADIENT`   | Gradient        |
| `CG_PAINT_TYPE_TEXTURE`    | Texture/image   |

## Object Structures

### cg_surface_t

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

### cg_path_t

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

A vector path composed of sub-paths (contours). Each sub-path begins with a `MOVE_TO` and optionally ends with `CLOSE`. Quadratic Beziers are stored as cubic. The quadratic-to-cubic conversion formula is:

```
CP1 = 2/3 * Q1 + 1/3 * P0
CP2 = 2/3 * Q1 + 1/3 * P2
```

### cg_paint_t / cg_solid_paint_t / cg_gradient_paint_t / cg_texture_paint_t

Paint is the "what to draw" — it can be a solid color, gradient, or texture. The `cg_paint_t` struct serves as a base with a `type` discriminator. Access the concrete type through the appropriate subtype pointer.

**Gradient paint** stores color stops inline (allocated in the same block as the struct). `values[6]` stores geometry:
- Linear: `{x1, y1, x2, y2}`
- Radial: `{cx0, cy0, r0, cx1, cy1, r1}` (start circle, then end circle)

**Texture paint** references a surface and stores opacity, texture type, and an affine matrix.
