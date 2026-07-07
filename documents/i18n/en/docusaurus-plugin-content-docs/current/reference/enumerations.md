# Enumerations Reference

This document summarizes all enumeration types in libcg for quick reference.

## cg_path_command_t

Path element type, used internally.

| Value                       | Points | Description                            |
|-----------------------------|--------|----------------------------------------|
| `CG_PATH_COMMAND_MOVE_TO`   | 1      | Move to a new point, start a subpath   |
| `CG_PATH_COMMAND_LINE_TO`   | 1      | Straight line to the target point      |
| `CG_PATH_COMMAND_CUBIC_TO`  | 3      | Cubic Bezier curve                     |
| `CG_PATH_COMMAND_CLOSE`     | 1      | Close the current subpath              |

## cg_spread_method_t

Gradient extension modes.

| Value                       | Description                              |
|-----------------------------|------------------------------------------|
| `CG_SPREAD_METHOD_PAD`      | Clamp to the nearest edge color          |
| `CG_SPREAD_METHOD_REFLECT`  | Mirror the gradient at each integer bound|
| `CG_SPREAD_METHOD_REPEAT`   | Repeat the gradient (wrap)               |

## cg_gradient_type_t

| Value                       | Description      |
|-----------------------------|------------------|
| `CG_GRADIENT_TYPE_LINEAR`   | Linear gradient  |
| `CG_GRADIENT_TYPE_RADIAL`   | Radial gradient  |

## cg_texture_type_t

| Value                     | Description                                          |
|---------------------------|------------------------------------------------------|
| `CG_TEXTURE_TYPE_PLAIN`   | Draw texture once (transparent outside the region)   |
| `CG_TEXTURE_TYPE_TILED`   | Repeat the texture to fill the entire drawing region |

## cg_line_cap_t

| Value                    | Description                                            |
|--------------------------|--------------------------------------------------------|
| `CG_LINE_CAP_BUTT`       | Butt cap, ends exactly at the endpoint (default)       |
| `CG_LINE_CAP_ROUND`      | Semicircular cap, extends past the endpoint            |
| `CG_LINE_CAP_SQUARE`     | Square cap, extends past the endpoint by half the width|

## cg_line_join_t

| Value                      | Description                                  |
|---------------------------|----------------------------------------------|
| `CG_LINE_JOIN_MITER`      | Sharp corner, clipped by miter limit (default)|
| `CG_LINE_JOIN_ROUND`      | Round corner                                 |
| `CG_LINE_JOIN_BEVEL`      | Beveled corner                               |

## cg_fill_rule_t

| Value                       | Description                  |
|-----------------------------|------------------------------|
| `CG_FILL_RULE_NON_ZERO`     | Non-zero winding rule (default) |
| `CG_FILL_RULE_EVEN_ODD`     | Even-odd rule                |

## cg_paint_type_t

| Value                       | Description      |
|-----------------------------|------------------|
| `CG_PAINT_TYPE_COLOR`       | Solid color      |
| `CG_PAINT_TYPE_GRADIENT`    | Gradient         |
| `CG_PAINT_TYPE_TEXTURE`     | Texture / image  |

## cg_operator_t

See the [Compositing Operators Reference](./operators).
