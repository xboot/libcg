# 架构概述

libcg 采用有状态的绘图模型。核心对象是 `cg_ctx_t`（绘图上下文），它持有：

- 对目标 `cg_surface_t` 像素缓冲区的引用
- 正在构建的当前 `cg_path_t`
- `cg_state_t` 对象的栈（通过 `cg_save` / `cg_restore` 压入/弹出）

每个状态保存：当前 paint（纯色、渐变或纹理）、变换矩阵、描边样式（线宽、线帽、连接、斜接限制、虚线模式）、填充规则、合成操作符、全局不透明度以及裁剪区域。

## 内存管理

Surface、Paint 和 Path 对象使用引用计数。接受这些对象的函数会在内部按需增加引用计数（例如 `cg_set_source` / `cg_paint_create_texture` 内部调用 `cg_paint_reference` 和 `cg_surface_reference`）。每个 `_create` 必须配以对应的 `_destroy`。

## 默认状态值

创建上下文时，各属性默认值如下：

| 属性         | 默认值                        |
|-------------|------------------------------|
| Paint       | `NULL`（解释为 `state->color`） |
| 颜色         | `{0, 0, 0, 1}`（黑色）        |
| 矩阵         | 单位矩阵                      |
| 线宽         | `1.0`                        |
| 线帽         | `CG_LINE_CAP_BUTT`           |
| 线段连接     | `CG_LINE_JOIN_MITER`         |
| 斜接限制     | `10.0`                       |
| 填充规则     | `CG_FILL_RULE_NON_ZERO`      |
| 操作符       | `CG_OPERATOR_SRC_OVER`       |
| 不透明度     | `1.0`                        |

## 颜色与坐标系

**颜色分量值**在整个 API 中均为 `[0.0, 1.0]` 范围内的浮点数。库内部会将值钳制到此范围。

**坐标系**：原点在左上角，X 向右递增，Y 向下递增。角度以弧度为单位，正方向为顺时针。

## 渲染管线

libcg 的渲染流程为：

1. 路径构建 — 通过 `cg_move_to`、`cg_line_to`、`cg_cubic_to` 等函数构建矢量路径；
2. 光栅化 — 路径被转换为 FreeType outline，再光栅化为带覆盖率信息的 span；
3. 裁剪交集 — 若设置了裁剪区域，span 与裁剪 span 取交集；
4. 合成混合 — 根据 paint 类型（纯色/渐变/纹理）采样颜色，用当前操作符和不透明度混合到 surface。

```
路径 → FT outline → 光栅化为 span → 与裁剪 span 取交集 → 用操作符/不透明度混合
```

## 接下来

- [数据类型](./data-types)：了解所有结构体与枚举的定义
- [矩阵函数](./matrix)：了解坐标变换
- [Surface 函数](./surface)：了解像素画布
- [Paint 函数](./paint)：了解纯色、渐变与纹理
- [Path 函数](./path)：了解路径构建
- [Context 函数](./context)：了解绘图上下文的所有操作
