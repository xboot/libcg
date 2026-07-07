# Basic Shapes

This example demonstrates how to use libcg to draw basic shapes: arcs, curves, lines, and rounded rectangles.

## Arc

Use `cg_arc` to draw an arc, combined with `cg_stroke` for stroking and `cg_fill` to fill the endpoint markers.

![arc](/screenshots/arc.png)

```c
struct cg_surface_t * surface = cg_surface_create(256, 256);
struct cg_ctx_t * ctx = cg_create(surface);

float xc = 128.0;
float yc = 128.0;
float radius = 100.0;
float angle1 = 45.0 * (M_PI / 180.0);
float angle2 = 180.0 * (M_PI / 180.0);

/* Draw the arc */
cg_set_line_width(ctx, 10.0);
cg_arc(ctx, xc, yc, radius, angle1, angle2);
cg_stroke(ctx);

/* Draw the center marker */
cg_set_source_rgba(ctx, 1, 0.2, 0.2, 0.6);
cg_set_line_width(ctx, 6.0);
cg_arc(ctx, xc, yc, 10.0, 0, 2 * M_PI);
cg_fill(ctx);

/* Draw the radius lines */
cg_arc(ctx, xc, yc, radius, angle1, angle1);
cg_line_to(ctx, xc, yc);
cg_arc(ctx, xc, yc, radius, angle2, angle2);
cg_line_to(ctx, xc, yc);
cg_stroke(ctx);

cg_destroy(ctx);
cg_surface_destroy(surface);
```

## Counter-clockwise Arc

`cg_arc_negative` draws an arc in the counter-clockwise direction:

![arc_negative](/screenshots/arc_negative.png)

```c
cg_arc_negative(ctx, xc, yc, radius, angle1, angle2);
cg_stroke(ctx);
```

## Cubic Bezier Curve

Use `cg_cubic_to` to draw a cubic Bezier curve, with the control point lines annotated:

![curve_to](/screenshots/curve_to.png)

```c
float x = 25.6, y = 128.0;
float x1 = 102.4, y1 = 230.4;
float x2 = 153.6, y2 = 25.6;
float x3 = 230.4, y3 = 128.0;

cg_move_to(ctx, x, y);
cg_cubic_to(ctx, x1, y1, x2, y2, x3, y3);

cg_set_line_width(ctx, 10.0);
cg_stroke(ctx);

/* Draw the control point lines */
cg_set_source_rgba(ctx, 1, 0.2, 0.2, 0.6);
cg_set_line_width(ctx, 6.0);
cg_move_to(ctx, x, y);
cg_line_to(ctx, x1, y1);
cg_move_to(ctx, x2, y2);
cg_line_to(ctx, x3, y3);
cg_stroke(ctx);
```

## Lines with Varying Width

Draw multiple lines with different widths:

![lines](/screenshots/lines.png)

```c
for(int i = 1; i < 16; i++)
{
    cg_set_line_cap(ctx, CG_LINE_CAP_ROUND);
    cg_set_line_width(ctx, i * 0.5);
    cg_set_source_rgb(ctx, 0, 0, 0);
    cg_move_to(ctx, 16 * i + 8, 16);
    cg_line_to(ctx, 16 * i - 8, 240);
    cg_stroke(ctx);
}
```

## Rounded Rectangle

Draw a rounded rectangle by joining arcs together:

![rounded_rectangle](/screenshots/rounded_rectangle.png)

```c
float x = 25.6, y = 25.6;
float width = 204.8, height = 204.8;
float radius = height / 10.0;
float degrees = M_PI / 180.0;

cg_new_path(ctx);
cg_arc(ctx, x + width - radius, y + radius, radius, -90 * degrees, 0 * degrees);
cg_arc(ctx, x + width - radius, y + height - radius, radius, 0 * degrees, 90 * degrees);
cg_arc(ctx, x + radius, y + height - radius, radius, 90 * degrees, 180 * degrees);
cg_arc(ctx, x + radius, y + radius, radius, 180 * degrees, 270 * degrees);
cg_close_path(ctx);

cg_set_source_rgb(ctx, 0.5, 0.5, 1);
cg_fill_preserve(ctx);
cg_set_source_rgba(ctx, 0.5, 0, 0, 0.5);
cg_set_line_width(ctx, 10.0);
cg_stroke(ctx);
```

## Curve Rectangle

Use cubic Bezier curves to draw a rectangle with rounded corners:

![curve_rectangle](/screenshots/curve_rectangle.png)

```c
float x0 = 25.6, y0 = 25.6;
float rect_width = 204.8, rect_height = 204.8;
float radius = 102.4;
float x1 = x0 + rect_width;
float y1 = y0 + rect_height;

cg_move_to(ctx, x0, y0 + radius);
cg_cubic_to(ctx, x0, y0, x0, y0, x0 + radius, y0);
cg_line_to(ctx, x1 - radius, y0);
cg_cubic_to(ctx, x1, y0, x1, y0, x1, y0 + radius);
cg_line_to(ctx, x1, y1 - radius);
cg_cubic_to(ctx, x1, y1, x1, y1, x1 - radius, y1);
cg_line_to(ctx, x0 + radius, y1);
cg_cubic_to(ctx, x0, y1, x0, y1, x0, y1 - radius);
cg_close_path(ctx);

cg_set_source_rgb(ctx, 0.5, 0.5, 1);
cg_fill_preserve(ctx);
cg_set_source_rgba(ctx, 0.5, 0, 0, 0.5);
cg_set_line_width(ctx, 10.0);
cg_stroke(ctx);
```
