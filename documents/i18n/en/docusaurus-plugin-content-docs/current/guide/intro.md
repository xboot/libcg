# About libcg

libcg is a lightweight 2D computer graphics C library whose drawing model is inspired by Cairo / HTML Canvas. It renders anti-aliased graphics into an RGBA32 pixel buffer through span-based rasterization.

## Features

- Pure C implementation, no external dependencies (other than libc and libm);
- Span-based anti-aliased rasterization, output in premultiplied RGBA32 format;
- Path construction: move_to, line_to, quad_to, cubic_to, arc_to, plus predefined shapes such as rectangles, rounded rectangles, ellipses, circles, and arcs;
- Fill and stroke: supports line width, line caps, joins, miter limit, and dash patterns;
- Gradient drawing: linear and radial gradients, with PAD / REFLECT / REPEAT extension modes;
- Texture drawing: supports both plain and tiled texture modes, with bilinear and nearest-neighbor sampling;
- Affine transforms: translate, scale, rotate, shear, with a save/restore state stack;
- Clipping: path-based clipping regions, supporting the intersection of multiple clips;
- Masking: uses the alpha channel of a paint or surface as a per-pixel mask;
- Compositing operators: a complete set of Porter-Duff compositing operators (CLEAR, SRC, DST, OVER, IN, OUT, ATOP, XOR, etc.);
- Reference-counted memory management, allowing objects to be reused across contexts.

## Design Philosophy

libcg adopts a stateful drawing model. The core object is the drawing context `cg_ctx_t`, which holds a reference to the target surface, the current path, and a state stack. All drawing operations are performed through the context, and state can be pushed/popped via `cg_save` / `cg_restore`.

The library's .c and .h files can be dropped directly into a project and compiled together, with no need for a complex build system.

## Related Links

- Source repository: [https://github.com/xboot/libcg](https://github.com/xboot/libcg)
- Online documentation: [https://libcg.xboot.org](https://libcg.xboot.org)
