# 默认状态值

创建绘图上下文 `cg_ctx_t` 时，各属性的默认值如下：

| 属性         | 默认值                        | 设置函数                |
|-------------|------------------------------|------------------------|
| Paint       | `NULL`（解释为 `state->color`） | `cg_set_source`        |
| 颜色         | `{0, 0, 0, 1}`（黑色）        | `cg_set_source_rgba`   |
| 矩阵         | 单位矩阵                      | `cg_set_matrix`        |
| 线宽         | `1.0`                        | `cg_set_line_width`    |
| 线帽         | `CG_LINE_CAP_BUTT`           | `cg_set_line_cap`      |
| 线段连接     | `CG_LINE_JOIN_MITER`         | `cg_set_line_join`     |
| 斜接限制     | `10.0`                       | `cg_set_miter_limit`   |
| 填充规则     | `CG_FILL_RULE_NON_ZERO`      | `cg_set_fill_rule`     |
| 操作符       | `CG_OPERATOR_SRC_OVER`       | `cg_set_operator`      |
| 不透明度     | `1.0`                        | `cg_set_opacity`       |
| 虚线         | 无（`ndashes = 0`）           | `cg_set_dash`          |
| 裁剪区域     | 整个 surface                  | `cg_clip` / `cg_reset_clip` |

## 说明

- **Paint 为 NULL 时**：使用状态中存储的颜色（`state->color`）作为绘制源。调用 `cg_set_source_rgb` / `cg_set_source_rgba` 会设置颜色并将 paint 设为 `NULL`。
- **裁剪区域**：初始为整个 surface `{0, 0, width, height}`，`cg_clip` 会将其缩小到路径与当前裁剪的交集，`cg_reset_clip` 恢复为整个 surface。
- **状态栈**：`cg_save` 压入当前状态的副本，`cg_restore` 弹出并恢复。初始状态永远不会被弹出。

## 像素格式

Surface 像素以**预乘 RGBA32** 格式存储：

- 每像素 4 字节，行优先排列
- `stride = width * 4`
- 颜色分量已预乘 alpha：`R' = R * A / 255`

## 坐标系

- 原点在左上角
- X 向右递增，Y 向下递增
- 角度以弧度为单位，正方向为顺时针
