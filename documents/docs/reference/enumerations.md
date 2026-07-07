# 枚举类型速查

本文档汇总 libcg 中所有枚举类型，方便快速查阅。

## cg_path_command_t

路径元素类型，内部使用。

| 值                          | 点数 | 描述                      |
|-----------------------------|------|---------------------------|
| `CG_PATH_COMMAND_MOVE_TO`   | 1    | 移动到新点，开始一个子路径   |
| `CG_PATH_COMMAND_LINE_TO`   | 1    | 直线到目标点               |
| `CG_PATH_COMMAND_CUBIC_TO`  | 3    | 三次贝塞尔曲线             |
| `CG_PATH_COMMAND_CLOSE`     | 1    | 闭合当前子路径             |

## cg_spread_method_t

渐变扩展模式。

| 值                          | 描述                      |
|-----------------------------|---------------------------|
| `CG_SPREAD_METHOD_PAD`      | 钳制到最近的边缘颜色        |
| `CG_SPREAD_METHOD_REFLECT`  | 在每个整数边界处镜像渐变     |
| `CG_SPREAD_METHOD_REPEAT`   | 重复渐变（环绕）            |

## cg_gradient_type_t

| 值                          | 描述     |
|-----------------------------|---------|
| `CG_GRADIENT_TYPE_LINEAR`   | 线性渐变 |
| `CG_GRADIENT_TYPE_RADIAL`   | 径向渐变 |

## cg_texture_type_t

| 值                         | 描述                                          |
|---------------------------|-----------------------------------------------|
| `CG_TEXTURE_TYPE_PLAIN`   | 单次绘制纹理（区域外为透明）                      |
| `CG_TEXTURE_TYPE_TILED`   | 纹理重复以填充整个绘制区域                        |

## cg_line_cap_t

| 值                       | 描述                                |
|--------------------------|-------------------------------------|
| `CG_LINE_CAP_BUTT`       | 平头端点，恰好在端点处结束（默认）      |
| `CG_LINE_CAP_ROUND`      | 半圆形端点，超出端点延伸              |
| `CG_LINE_CAP_SQUARE`     | 方形端点，超出端点延伸半个线宽         |

## cg_line_join_t

| 值                        | 描述                                  |
|---------------------------|---------------------------------------|
| `CG_LINE_JOIN_MITER`      | 尖角，受斜接限制裁剪（默认）             |
| `CG_LINE_JOIN_ROUND`      | 圆角                                  |
| `CG_LINE_JOIN_BEVEL`      | 平角（斜角）                           |

## cg_fill_rule_t

| 值                          | 描述                    |
|-----------------------------|------------------------|
| `CG_FILL_RULE_NON_ZERO`     | 非零环绕规则（默认）      |
| `CG_FILL_RULE_EVEN_ODD`     | 奇偶规则                |

## cg_paint_type_t

| 值                          | 描述         |
|----------------------------|-------------|
| `CG_PAINT_TYPE_COLOR`      | 纯色         |
| `CG_PAINT_TYPE_GRADIENT`   | 渐变         |
| `CG_PAINT_TYPE_TEXTURE`    | 纹理/图像    |

## cg_operator_t

详见[合成操作符参考](./operators)。
