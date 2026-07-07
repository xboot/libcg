# 矩阵函数

矩阵将坐标从用户空间变换到 surface 空间。所有矩阵函数都在 `m` 指向的矩阵上原地操作。

## cg_matrix_init

```c
void cg_matrix_init(struct cg_matrix_t * m, float a, float b, float c, float d, float tx, float ty);
```

用显式值初始化矩阵。

## cg_matrix_init_identity

```c
void cg_matrix_init_identity(struct cg_matrix_t * m);
```

将矩阵设置为单位矩阵：`{1, 0, 0, 1, 0, 0}`。

## cg_matrix_init_translate

```c
void cg_matrix_init_translate(struct cg_matrix_t * m, float tx, float ty);
```

初始化为纯平移矩阵：`{1, 0, 0, 1, tx, ty}`。

## cg_matrix_init_scale

```c
void cg_matrix_init_scale(struct cg_matrix_t * m, float sx, float sy);
```

初始化为纯缩放矩阵：`{sx, 0, 0, sy, 0, 0}`。

## cg_matrix_init_rotate

```c
void cg_matrix_init_rotate(struct cg_matrix_t * m, float r);
```

初始化为旋转 `r` 弧度的旋转矩阵：`{cos(r), sin(r), -sin(r), cos(r), 0, 0}`。

## cg_matrix_init_shear

```c
void cg_matrix_init_shear(struct cg_matrix_t * m, float shx, float shy);
```

初始化为剪切矩阵：`{1, shy, shx, 1, 0, 0}`。

## cg_matrix_translate

```c
void cg_matrix_translate(struct cg_matrix_t * m, float tx, float ty);
```

在现有矩阵上后乘平移变换：
```
tx' = tx + a*dx + c*dy
ty' = ty + b*dx + d*dy
```

## cg_matrix_scale

```c
void cg_matrix_scale(struct cg_matrix_t * m, float sx, float sy);
```

后乘缩放变换：`a*=sx; b*=sx; c*=sy; d*=sy`。

## cg_matrix_rotate

```c
void cg_matrix_rotate(struct cg_matrix_t * m, float r);
```

后乘旋转 `r` 弧度的旋转变换。

## cg_matrix_shear

```c
void cg_matrix_shear(struct cg_matrix_t * m, float shx, float shy);
```

后乘剪切变换：`m = m * shear(shx, shy)`。

## cg_matrix_multiply

```c
void cg_matrix_multiply(struct cg_matrix_t * m, struct cg_matrix_t * m1, struct cg_matrix_t * m2);
```

设置 `m = m1 * m2`。当两个矩阵都是轴对齐（即 `b == 0` 且 `c == 0`）时，使用优化路径。

## cg_matrix_invert

```c
int cg_matrix_invert(struct cg_matrix_t * m);
```

原地求逆矩阵。成功返回非零值，矩阵奇异时返回 `0`。

特殊情况：
- 如果 `b == 0` 且 `c == 0`（轴对齐）：若 `a == 0` 或 `d == 0` 则返回 `0`。
- 否则：若 `|det| <= 1e-20` 则返回 `0`。
