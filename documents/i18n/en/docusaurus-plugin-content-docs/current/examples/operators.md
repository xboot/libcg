# Compositing Operators

This example demonstrates libcg's Porter-Duff compositing operators, which control how new pixels are blended with existing content.

## Operators Overview

The following code draws the effects of all 12 compositing operators. In each cell, the blue circle is the existing content and the red circle is the newly drawn content:

![operators](/screenshots/operators.png)

```c
static void draw_operator_cell(struct cg_ctx_t * ctx, float x, float y,
    float w, float h, enum cg_operator_t op)
{
    float cx = x + w * 0.5;
    float cy = y + h * 0.55;
    float r = h * 0.28;

    cg_save(ctx);
    cg_rectangle(ctx, x, y, w, h);
    cg_clip(ctx);

    /* Checkerboard background */
    cg_set_source_rgb(ctx, 0.92, 0.92, 0.92);
    cg_rectangle(ctx, x, y, w, h);
    cg_fill(ctx);
    cg_set_source_rgb(ctx, 0.78, 0.78, 0.78);
    cg_rectangle(ctx, x, y, w * 0.5, h * 0.5);
    cg_fill(ctx);
    cg_rectangle(ctx, x + w * 0.5, y + h * 0.5, w * 0.5, h * 0.5);
    cg_fill(ctx);

    /* Blue circle (destination) */
    cg_set_source_rgba(ctx, 0.0, 0.0, 1.0, 0.7);
    cg_arc(ctx, cx - w * 0.12, cy, r, 0, 2 * M_PI);
    cg_fill(ctx);

    /* Red circle (source), using the specified operator */
    cg_set_operator(ctx, op);
    cg_set_source_rgba(ctx, 1.0, 0.0, 0.0, 0.7);
    cg_arc(ctx, cx + w * 0.12, cy, r, 0, 2 * M_PI);
    cg_fill(ctx);

    cg_restore(ctx);

    /* Cell border */
    cg_set_line_width(ctx, 1.0);
    cg_set_source_rgba(ctx, 0.0, 0.0, 0.0, 0.35);
    cg_rectangle(ctx, x + 0.5, y + 0.5, w - 1, h - 1);
    cg_stroke(ctx);
}

/* Draw the 12 operators */
enum cg_operator_t ops[] = {
    CG_OPERATOR_CLEAR,
    CG_OPERATOR_SRC,
    CG_OPERATOR_DST,
    CG_OPERATOR_SRC_OVER,
    CG_OPERATOR_DST_OVER,
    CG_OPERATOR_SRC_IN,
    CG_OPERATOR_DST_IN,
    CG_OPERATOR_SRC_OUT,
    CG_OPERATOR_DST_OUT,
    CG_OPERATOR_SRC_ATOP,
    CG_OPERATOR_DST_ATOP,
    CG_OPERATOR_XOR,
};

for(int i = 0; i < 12; i++)
{
    float x0 = (float)((i % 3) * 256) / 3.0;
    float x1 = (float)(((i % 3) + 1) * 256) / 3.0;
    float y = (float)((i / 3) * 64);
    draw_operator_cell(ctx, x0, y, x1 - x0, 64.0, ops[i]);
}
```

## Operator Description

For detailed formulas and descriptions, see the [Compositing Operators Reference](../reference/operators).

:::note
When the paint is a solid color with opacity=255 and the operator is `SRC_OVER`, the library internally optimizes by using the `SRC` operator for better performance.
:::
