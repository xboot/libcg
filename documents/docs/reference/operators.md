# 合成操作符参考

libcg 支持完整的 Porter-Duff 合成操作符，控制新像素（源）如何与已有内容（目标）混合。

## 操作符列表

| 值                     | 公式                          | 效果                        |
|------------------------|-------------------------------|----------------------------|
| `CG_OPERATOR_CLEAR`    | 0                              | 清除目标                    |
| `CG_OPERATOR_SRC`      | S                              | 复制源                      |
| `CG_OPERATOR_DST`      | D                              | 保持目标不变                |
| `CG_OPERATOR_SRC_OVER` | S + D*(1 - Aₛ)                | 源在上（默认）              |
| `CG_OPERATOR_DST_OVER` | D + S*(1 - Aᴅ)                | 目标在上                    |
| `CG_OPERATOR_SRC_IN`   | S * Aᴅ                         | 目标存在处显示源             |
| `CG_OPERATOR_DST_IN`   | D * Aₛ                         | 源存在处显示目标             |
| `CG_OPERATOR_SRC_OUT`  | S * (1 - Aᴅ)                   | 目标不存在处显示源           |
| `CG_OPERATOR_DST_OUT`  | D * (1 - Aₛ)                   | 源不存在处显示目标           |
| `CG_OPERATOR_SRC_ATOP` | S*Aᴅ + D*(1 - Aₛ)             | 源在目标之上，裁剪           |
| `CG_OPERATOR_DST_ATOP` | D*Aₛ + S*(1 - Aᴅ)             | 目标在源之上，裁剪           |
| `CG_OPERATOR_XOR`      | S*(1 - Aᴅ) + D*(1 - Aₛ)       | 非重叠区域合并               |

其中 S=源, D=目标, Aₛ=源透明度, Aᴅ=目标透明度。

## 使用方法

通过 `cg_set_operator` 设置当前合成操作符：

```c
cg_set_operator(ctx, CG_OPERATOR_SRC_OVER);
cg_set_source_rgba(ctx, 1.0, 0.0, 0.0, 0.7);
cg_arc(ctx, cx, cy, r, 0, 2 * M_PI);
cg_fill(ctx);
```

操作符影响所有渲染操作：`cg_fill`、`cg_stroke`、`cg_paint`、`cg_mask` 等。

## 性能优化

当 paint 为透明度=255 的纯色且操作符为 `SRC_OVER` 时，库内部会自动优化为使用 `SRC` 操作符，避免不必要的 alpha 混合计算。

## 效果示例

参见[合成操作符示例](../examples/operators)查看所有 12 种操作符的视觉效果。
