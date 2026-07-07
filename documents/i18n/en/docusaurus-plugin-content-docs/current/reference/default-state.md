# Default State Values

When the drawing context `cg_ctx_t` is created, the default values of each attribute are as follows:

| Attribute     | Default Value                   | Setter Function         |
|---------------|---------------------------------|-------------------------|
| Paint         | `NULL` (interpreted as `state->color`) | `cg_set_source`         |
| Color         | `{0, 0, 0, 1}` (black)          | `cg_set_source_rgba`    |
| Matrix        | Identity matrix                 | `cg_set_matrix`         |
| Line width    | `1.0`                           | `cg_set_line_width`     |
| Line cap      | `CG_LINE_CAP_BUTT`              | `cg_set_line_cap`       |
| Line join     | `CG_LINE_JOIN_MITER`            | `cg_set_line_join`      |
| Miter limit   | `10.0`                          | `cg_set_miter_limit`    |
| Fill rule     | `CG_FILL_RULE_NON_ZERO`         | `cg_set_fill_rule`      |
| Operator      | `CG_OPERATOR_SRC_OVER`          | `cg_set_operator`       |
| Opacity       | `1.0`                           | `cg_set_opacity`        |
| Dash          | None (`ndashes = 0`)            | `cg_set_dash`           |
| Clip region   | The entire surface              | `cg_clip` / `cg_reset_clip` |

## Notes

- **When Paint is NULL**: the color stored in the state (`state->color`) is used as the drawing source. Calling `cg_set_source_rgb` / `cg_set_source_rgba` sets the color and resets paint to `NULL`.
- **Clip region**: initially the entire surface `{0, 0, width, height}`. `cg_clip` shrinks it to the intersection of the path and the current clip, while `cg_reset_clip` restores it to the entire surface.
- **State stack**: `cg_save` pushes a copy of the current state, and `cg_restore` pops and restores it. The initial state is never popped.

## Pixel Format

Surface pixels are stored in **premultiplied RGBA32** format:

- 4 bytes per pixel, arranged in row-major order
- `stride = width * 4`
- Color components are premultiplied by alpha: `R' = R * A / 255`

## Coordinate System

- The origin is at the top-left corner
- X increases to the right, Y increases downward
- Angles are in radians, with the positive direction being clockwise
