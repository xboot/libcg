# Path Functions

Path objects can be built independently of a context and reused. A path is composed of sub-paths (contours), each beginning with a `MOVE_TO` and optionally ending with `CLOSE`.

## Lifecycle

### cg_path_create

```c
struct cg_path_t * cg_path_create(void);
```

Creates an empty path with no sub-paths.

### cg_path_destroy

```c
void cg_path_destroy(struct cg_path_t * path);
```

Decrements the reference count, freeing path data when it reaches zero.

### cg_path_reference

```c
struct cg_path_t * cg_path_reference(struct cg_path_t * path);
```

Increments the reference count and returns the path.

## Path Manipulation

### cg_path_reset

```c
void cg_path_reset(struct cg_path_t * path);
```

Clears all elements, contour data, and curve counts. Resets to an empty path.

### cg_path_sub_path

```c
void cg_path_sub_path(struct cg_path_t * path);
```

Marks that the next drawing command should begin a new sub-path. If the next command is `line_to`, `cubic_to`, etc. without a preceding `move_to`, it will be promoted to a `move_to`.

### cg_path_close

```c
void cg_path_close(struct cg_path_t * path);
```

Closes the current sub-path by adding a `CLOSE` command referencing the sub-path's start point. Does nothing if the path is empty. Resets the `sub_path` flag.

### cg_path_transform

```c
void cg_path_transform(struct cg_path_t * path, struct cg_matrix_t * m);
```

Transforms all points in the path by matrix `m` in place. Traverses all move, line, close (1 point) and cubic (3 points) commands and maps each point through the matrix.

### cg_path_get_current_point

```c
void cg_path_get_current_point(struct cg_path_t * path, float * x, float * y);
```

Returns the current position. If there are no points or the path is in "new sub-path" mode, returns `(0, 0)`. Either `x` or `y` may be `NULL` if only one coordinate is needed.

## Building Paths

Paths are built incrementally. If the path is empty, the first drawing command implicitly creates a `MOVE_TO` to `(0, 0)`.

### cg_path_move_to

```c
void cg_path_move_to(struct cg_path_t * path, float x, float y);
```

Starts a new sub-path at `(x, y)`. Increments the contour count. Resets the `sub_path` flag.

### cg_path_line_to

```c
void cg_path_line_to(struct cg_path_t * path, float x, float y);
```

Adds a straight line segment from the current point to `(x, y)`.

- If `sub_path` is set, promotes to `move_to(x, y)`.
- If the path is empty, first adds `move_to(0, 0)`.

### cg_path_quad_to

```c
void cg_path_quad_to(struct cg_path_t * path, float x1, float y1, float x2, float y2);
```

Adds a quadratic Bezier curve. Internally converted to cubic:
```
CP1 = 2/3 * (x1, y1) + 1/3 * current_point
CP2 = 2/3 * (x1, y1) + 1/3 * (x2, y2)
```

If the path is empty, first adds `move_to(0, 0)`.

### cg_path_cubic_to

```c
void cg_path_cubic_to(struct cg_path_t * path,
    float x1, float y1,
    float x2, float y2,
    float x3, float y3);
```

Adds a cubic Bezier curve with two control points `(x1, y1)`, `(x2, y2)` and end point `(x3, y3)`. Increments the curve count.

If the path is empty or `sub_path` is set, first adds `move_to(0, 0)`.

### cg_path_arc_to

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

## Predefined Shapes

### cg_path_add_rectangle

```c
void cg_path_add_rectangle(struct cg_path_t * path, float x, float y, float w, float h);
```

Adds a closed rectangle as a new sub-path: `move_to(x, y)` → `line_to(x+w, y)` → `line_to(x+w, y+h)` → `line_to(x, y+h)` → `line_to(x, y)` → `close`.

### cg_path_add_round_rectangle

```c
void cg_path_add_round_rectangle(struct cg_path_t * path,
    float x, float y,
    float w, float h,
    float rx, float ry);
```

Adds a closed rounded rectangle. Corner radii are clamped to `min(w/2, h/2)`. If both radii are zero, falls back to `cg_path_add_rectangle`. Uses cubic Beziers with control points at `radius * 0.5522847498` (KAPPA) to approximate quarter-circles.

### cg_path_add_ellipse

```c
void cg_path_add_ellipse(struct cg_path_t * path, float cx, float cy, float rx, float ry);
```

Adds a closed ellipse centered at `(cx, cy)` with radii `(rx, ry)`. Uses 4 cubic Bezier curves per ellipse.

### cg_path_add_circle

```c
void cg_path_add_circle(struct cg_path_t * path, float cx, float cy, float r);
```

Convenience wrapper: calls `cg_path_add_ellipse(path, cx, cy, r, r)`.

### cg_path_add_arc

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

### cg_path_add_path

```c
void cg_path_add_path(struct cg_path_t * path,
    struct cg_path_t * source,
    struct cg_matrix_t * m);
```

Appends `source` to the current path. If `m` is `NULL`, the source elements are copied directly. If `m` is non-`NULL`, each point in `source` is transformed by `m` before being added.

### cg_path_extents

```c
float cg_path_extents(struct cg_path_t * path, struct cg_rect_t * extents, int tight);
```

Computes the bounding box of the path.

| Parameter | Description                                                              |
|-----------|--------------------------------------------------------------------------|
| `extents` | Output rectangle, or `NULL` if not needed                                |
| `tight`   | Non-zero: flattens Bezier curves for exact extents. 0: uses control points only |

**Returns**: The approximate arc length of the path (sum of segment lengths), not the area.
