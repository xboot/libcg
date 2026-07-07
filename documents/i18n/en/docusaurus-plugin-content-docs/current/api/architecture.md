# Architecture Overview

libcg follows a stateful drawing model. The central object is `cg_ctx_t` (the drawing context), which holds:

- A reference to the target `cg_surface_t` pixel buffer
- The current `cg_path_t` being constructed
- A stack of `cg_state_t` objects (push/pop via `cg_save` / `cg_restore`)

Each state stores: current paint (solid, gradient, or texture), transformation matrix, stroke style (line width, line cap, line join, miter limit, dash pattern), fill rule, compositing operator, global opacity, and the clip region.

## Memory Management

Surface, Paint, and Path objects use reference counting. Functions that accept these objects will increment the reference count as needed (e.g., `cg_set_source` / `cg_paint_create_texture` call `cg_paint_reference` and `cg_surface_reference` internally). Every `_create` must be paired with the corresponding `_destroy`.

## Default State Values

When a context is created, the default values of each property are:

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

## Color and Coordinate System

**Color component values** throughout the API are floats in the `[0.0, 1.0]` range. The library clamps values to this range internally.

**Coordinate system**: Origin at the top-left, X increases to the right, Y increases downward. Angles are in radians, with the positive direction being clockwise.

## Rendering Pipeline

The rendering flow of libcg is:

1. Path construction — build a vector path via functions such as `cg_move_to`, `cg_line_to`, `cg_cubic_to`, etc.;
2. Rasterization — the path is converted to a FreeType outline, then rasterized into spans with coverage information;
3. Clip intersection — if a clip region is set, the spans are intersected with the clip spans;
4. Compositing and blending — colors are sampled according to the paint type (solid/gradient/texture) and blended onto the surface using the current operator and opacity.

```
Path → FT outline → Rasterize to spans → Intersect with clip spans → Blend with operator/opacity
```

## Next

- [Data Types](./data-types): Learn about all struct and enum definitions
- [Matrix Functions](./matrix): Learn about coordinate transformations
- [Surface Functions](./surface): Learn about the pixel canvas
- [Paint Functions](./paint): Learn about solid colors, gradients, and textures
- [Path Functions](./path): Learn about path construction
- [Context Functions](./context): Learn about all operations of the drawing context
