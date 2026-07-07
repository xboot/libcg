# 关于 libcg

libcg 是一个轻量级的 2D 计算机图形 C 库，其绘图模型借鉴自 Cairo / HTML Canvas。它通过基于 span 的光栅化将抗锯齿图形渲染到 RGBA32 像素缓冲区中。

## 特性

- 纯 C 实现，无外部依赖（除 libc 和 libm）；
- 基于 span 的抗锯齿光栅化，输出预乘 RGBA32 格式；
- 路径构建：move_to、line_to、quad_to、cubic_to、arc_to，以及矩形、圆角矩形、椭圆、圆、圆弧等预定义形状；
- 填充与描边：支持线宽、线帽、连接、斜接限制、虚线模式；
- 渐变绘制：线性渐变与径向渐变，支持 PAD / REFLECT / REPEAT 三种扩展模式；
- 纹理绘制：支持普通与平铺两种纹理模式，含双线性与最近邻采样；
- 仿射变换：平移、缩放、旋转、剪切，支持 save/restore 状态栈；
- 裁剪：基于路径的裁剪区域，支持多次裁剪交集；
- 遮罩：使用 paint 或 surface 的 alpha 通道作为逐像素遮罩；
- 合成操作符：完整的 Porter-Duff 合成操作符集（CLEAR、SRC、DST、OVER、IN、OUT、ATOP、XOR 等）；
- 引用计数内存管理，支持对象跨上下文复用。

## 设计理念

libcg 采用有状态的绘图模型。核心对象是绘图上下文 `cg_ctx_t`，它持有目标 surface 引用、当前路径和状态栈。所有绘制操作通过上下文完成，状态可通过 `cg_save` / `cg_restore` 压栈/出栈。

库的 .c 和 .h 文件可以直接放入项目中一同编译，无需复杂的构建系统。

## 相关链接

- 源码仓库：[https://github.com/xboot/libcg](https://github.com/xboot/libcg)
- 在线文档：[https://libcg.xboot.org](https://libcg.xboot.org)
