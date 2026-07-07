# 合成操作符

本示例演示 libcg 的 Porter-Duff 合成操作符，控制新像素如何与已有内容混合。

## 操作符总览

以下代码绘制所有 12 种合成操作符的效果，每个单元格中蓝色圆为已有内容，红色圆为新绘制内容：

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

    /* 棋盘格背景 */
    cg_set_source_rgb(ctx, 0.92, 0.92, 0.92);
    cg_rectangle(ctx, x, y, w, h);
    cg_fill(ctx);
    cg_set_source_rgb(ctx, 0.78, 0.78, 0.78);
    cg_rectangle(ctx, x, y, w * 0.5, h * 0.5);
    cg_fill(ctx);
    cg_rectangle(ctx, x + w * 0.5, y + h * 0.5, w * 0.5, h * 0.5);
    cg_fill(ctx);

    /* 蓝色圆（目标） */
    cg_set_source_rgba(ctx, 0.0, 0.0, 1.0, 0.7);
    cg_arc(ctx, cx - w * 0.12, cy, r, 0, 2 * M_PI);
    cg_fill(ctx);

    /* 红色圆（源），使用指定操作符 */
    cg_set_operator(ctx, op);
    cg_set_source_rgba(ctx, 1.0, 0.0, 0.0, 0.7);
    cg_arc(ctx, cx + w * 0.12, cy, r, 0, 2 * M_PI);
    cg_fill(ctx);

    cg_restore(ctx);

    /* 单元格边框 */
    cg_set_line_width(ctx, 1.0);
    cg_set_source_rgba(ctx, 0.0, 0.0, 0.0, 0.35);
    cg_rectangle(ctx, x + 0.5, y + 0.5, w - 1, h - 1);
    cg_stroke(ctx);
}

/* 绘制 12 种操作符 */
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

## 操作符说明

详细公式与说明参见[合成操作符参考](../reference/operators)。

:::note
当 paint 为透明度=255 的纯色且操作符为 `SRC_OVER` 时，库内部会优化为使用 `SRC` 操作符，以获得更好的性能。
:::
