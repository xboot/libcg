# Compositing Operators Reference

libcg supports the complete set of Porter-Duff compositing operators, controlling how new pixels (source) are blended with existing content (destination).

## Operator List

| Value                   | Formula                        | Effect                              |
|-------------------------|--------------------------------|-------------------------------------|
| `CG_OPERATOR_CLEAR`     | 0                              | Clear the destination               |
| `CG_OPERATOR_SRC`       | S                              | Copy the source                     |
| `CG_OPERATOR_DST`       | D                              | Keep the destination unchanged      |
| `CG_OPERATOR_SRC_OVER`  | S + D*(1 - Aₛ)                | Source on top (default)             |
| `CG_OPERATOR_DST_OVER`  | D + S*(1 - Aᴅ)                | Destination on top                  |
| `CG_OPERATOR_SRC_IN`    | S * Aᴅ                         | Show source where destination exists|
| `CG_OPERATOR_DST_IN`    | D * Aₛ                         | Show destination where source exists|
| `CG_OPERATOR_SRC_OUT`   | S * (1 - Aᴅ)                   | Show source where destination is absent |
| `CG_OPERATOR_DST_OUT`   | D * (1 - Aₛ)                   | Show destination where source is absent |
| `CG_OPERATOR_SRC_ATOP`  | S*Aᴅ + D*(1 - Aₛ)             | Source on top of destination, clipped|
| `CG_OPERATOR_DST_ATOP`  | D*Aₛ + S*(1 - Aᴅ)             | Destination on top of source, clipped|
| `CG_OPERATOR_XOR`       | S*(1 - Aᴅ) + D*(1 - Aₛ)       | Combine the non-overlapping regions |

Where S=source, D=destination, Aₛ=source alpha, Aᴅ=destination alpha.

## Usage

Set the current compositing operator via `cg_set_operator`:

```c
cg_set_operator(ctx, CG_OPERATOR_SRC_OVER);
cg_set_source_rgba(ctx, 1.0, 0.0, 0.0, 0.7);
cg_arc(ctx, cx, cy, r, 0, 2 * M_PI);
cg_fill(ctx);
```

The operator affects all rendering operations: `cg_fill`, `cg_stroke`, `cg_paint`, `cg_mask`, etc.

## Performance Optimization

When the paint is a solid color with alpha=255 and the operator is `SRC_OVER`, the library automatically optimizes internally by using the `SRC` operator, avoiding unnecessary alpha blending computations.

## Visual Examples

See the [Compositing Operators Example](../examples/operators) for the visual effect of all 12 operators.
