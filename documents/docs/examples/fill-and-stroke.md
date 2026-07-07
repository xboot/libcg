# 填充与描边

本示例演示 libcg 的填充、描边、线帽、连接、虚线和填充规则等绘图样式。

## 填充与描边组合

使用 `cg_fill_preserve` 先填充再描边，保留路径供后续操作：

![fill_and_stroke](/screenshots/fill_and_stroke.png)

```c
/* 绘制三角形 */
cg_move_to(ctx, 128.0, 25.6);
cg_line_to(ctx, 230.4, 230.4);
cg_rel_line_to(ctx, -102.4, 0.0);
cg_cubic_to(ctx, 51.2, 230.4, 51.2, 128.0, 128.0, 128.0);
cg_close_path(ctx);

/* 绘制菱形 */
cg_move_to(ctx, 64.0, 25.6);
cg_rel_line_to(ctx, 51.2, 51.2);
cg_rel_line_to(ctx, -51.2, 51.2);
cg_rel_line_to(ctx, -51.2, -51.2);
cg_close_path(ctx);

/* 填充并描边 */
cg_set_line_width(ctx, 10.0);
cg_set_source_rgb(ctx, 0, 0, 1);
cg_fill_preserve(ctx);
cg_set_source_rgb(ctx, 0, 0, 0);
cg_stroke(ctx);
```

## 填充规则

对比 `CG_FILL_RULE_EVEN_ODD` 与 `CG_FILL_RULE_NON_ZERO` 的效果：

![fill_style](/screenshots/fill_style.png)

```c
cg_set_line_width(ctx, 6);

/* 上半部分：奇偶规则 */
cg_rectangle(ctx, 12, 12, 232, 70);
cg_new_sub_path(ctx);
cg_arc(ctx, 64, 64, 40, 0, 2 * M_PI);
cg_new_sub_path(ctx);
cg_arc_negative(ctx, 192, 64, 40, 0, -2 * M_PI);
cg_set_fill_rule(ctx, CG_FILL_RULE_EVEN_ODD);
cg_set_source_rgb(ctx, 0, 0.7, 0);
cg_fill_preserve(ctx);
cg_set_source_rgb(ctx, 0, 0, 0);
cg_stroke(ctx);

/* 下半部分：非零规则 */
cg_save(ctx);
cg_translate(ctx, 0, 128);
cg_rectangle(ctx, 12, 12, 232, 70);
cg_new_sub_path(ctx);
cg_arc(ctx, 64, 64, 40, 0, 2 * M_PI);
cg_new_sub_path(ctx);
cg_arc_negative(ctx, 192, 64, 40, 0, -2 * M_PI);
cg_set_fill_rule(ctx, CG_FILL_RULE_NON_ZERO);
cg_set_source_rgb(ctx, 0, 0, 0.9);
cg_fill_preserve(ctx);
cg_set_source_rgb(ctx, 0, 0, 0);
cg_stroke(ctx);
cg_restore(ctx);
```

## 线帽样式

对比三种线帽样式：`BUTT`、`ROUND`、`SQUARE`：

![set_line_cap](/screenshots/set_line_cap.png)

```c
cg_set_line_width(ctx, 30.0);

/* BUTT - 平头 */
cg_set_line_cap(ctx, CG_LINE_CAP_BUTT);
cg_move_to(ctx, 64.0, 50.0);
cg_line_to(ctx, 64.0, 200.0);
cg_stroke(ctx);

/* ROUND - 圆头 */
cg_set_line_cap(ctx, CG_LINE_CAP_ROUND);
cg_move_to(ctx, 128.0, 50.0);
cg_line_to(ctx, 128.0, 200.0);
cg_stroke(ctx);

/* SQUARE - 方头 */
cg_set_line_cap(ctx, CG_LINE_CAP_SQUARE);
cg_move_to(ctx, 192.0, 50.0);
cg_line_to(ctx, 192.0, 200.0);
cg_stroke(ctx);
```

## 线段连接

对比三种连接样式：`MITER`、`BEVEL`、`ROUND`：

![set_line_join](/screenshots/set_line_join.png)

```c
cg_set_line_width(ctx, 40.96);

/* MITER - 尖角 */
cg_move_to(ctx, 76.8, 84.48);
cg_rel_line_to(ctx, 51.2, -51.2);
cg_rel_line_to(ctx, 51.2, 51.2);
cg_set_line_join(ctx, CG_LINE_JOIN_MITER);
cg_stroke(ctx);

/* BEVEL - 平角 */
cg_move_to(ctx, 76.8, 161.28);
cg_rel_line_to(ctx, 51.2, -51.2);
cg_rel_line_to(ctx, 51.2, 51.2);
cg_set_line_join(ctx, CG_LINE_JOIN_BEVEL);
cg_stroke(ctx);

/* ROUND - 圆角 */
cg_move_to(ctx, 76.8, 238.08);
cg_rel_line_to(ctx, 51.2, -51.2);
cg_rel_line_to(ctx, 51.2, 51.2);
cg_set_line_join(ctx, CG_LINE_JOIN_ROUND);
cg_stroke(ctx);
```

## 多段线圆头端点

演示多条独立线段的圆头端点效果：

![multi_segment_caps](/screenshots/multi_segment_caps.png)

```c
cg_move_to(ctx, 50.0, 75.0);
cg_line_to(ctx, 200.0, 75.0);

cg_move_to(ctx, 50.0, 125.0);
cg_line_to(ctx, 200.0, 125.0);

cg_move_to(ctx, 50.0, 175.0);
cg_line_to(ctx, 200.0, 175.0);

cg_set_line_width(ctx, 30.0);
cg_set_line_cap(ctx, CG_LINE_CAP_ROUND);
cg_stroke(ctx);
```

## 虚线

使用 `cg_set_dash` 设置虚线模式：

![dash](/screenshots/dash.png)

```c
float dashes[] = { 50.0, 10.0, 10.0, 10.0 };
int ndash = sizeof(dashes) / sizeof(dashes[0]);
float offset = -50.0;

cg_set_dash(ctx, dashes, ndash, offset);
cg_set_line_width(ctx, 10.0);

cg_move_to(ctx, 128.0, 25.6);
cg_line_to(ctx, 230.4, 230.4);
cg_rel_line_to(ctx, -102.4, 0.0);
cg_cubic_to(ctx, 51.2, 230.4, 51.2, 128.0, 128.0, 128.0);
cg_stroke(ctx);
```

## 空心五角星

使用奇偶填充规则绘制空心五角星：

![hollow_star](/screenshots/hollow_star.png)

```c
float x[5], y[5];
for(int i = 0; i < 5; i++)
{
    float a = (-90.0 + i * 72.0) * (M_PI / 180.0);
    x[i] = 128.0 + cos(a) * 110.0;
    y[i] = 128.0 + sin(a) * 110.0;
}

for(int i = 0; i < 5; i++)
{
    int n = (i * 2) % 5;
    if(i == 0)
        cg_move_to(ctx, x[n], y[n]);
    else
        cg_line_to(ctx, x[n], y[n]);
}
cg_close_path(ctx);

cg_set_fill_rule(ctx, CG_FILL_RULE_EVEN_ODD);
cg_set_source_rgb(ctx, 1.0, 0.75, 0.0);
cg_fill_preserve(ctx);
cg_set_source_rgba(ctx, 0.0, 0.0, 0.0, 0.35);
cg_set_line_width(ctx, 5.0);
cg_stroke(ctx);
```
