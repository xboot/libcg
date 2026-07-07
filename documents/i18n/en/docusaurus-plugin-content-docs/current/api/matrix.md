# Matrix Functions

Matrices transform coordinates from user space to surface space. All matrix functions operate in-place on the matrix pointed to by `m`.

## cg_matrix_init

```c
void cg_matrix_init(struct cg_matrix_t * m, float a, float b, float c, float d, float tx, float ty);
```

Initialize a matrix with explicit values.

## cg_matrix_init_identity

```c
void cg_matrix_init_identity(struct cg_matrix_t * m);
```

Set the matrix to identity: `{1, 0, 0, 1, 0, 0}`.

## cg_matrix_init_translate

```c
void cg_matrix_init_translate(struct cg_matrix_t * m, float tx, float ty);
```

Initialize as a translation-only matrix: `{1, 0, 0, 1, tx, ty}`.

## cg_matrix_init_scale

```c
void cg_matrix_init_scale(struct cg_matrix_t * m, float sx, float sy);
```

Initialize as a scale-only matrix: `{sx, 0, 0, sy, 0, 0}`.

## cg_matrix_init_rotate

```c
void cg_matrix_init_rotate(struct cg_matrix_t * m, float r);
```

Initialize as a rotation matrix by `r` radians: `{cos(r), sin(r), -sin(r), cos(r), 0, 0}`.

## cg_matrix_init_shear

```c
void cg_matrix_init_shear(struct cg_matrix_t * m, float shx, float shy);
```

Initialize as a shear matrix: `{1, shy, shx, 1, 0, 0}`.

## cg_matrix_translate

```c
void cg_matrix_translate(struct cg_matrix_t * m, float tx, float ty);
```

Post-multiply a translation onto the existing matrix:
```
tx' = tx + a*dx + c*dy
ty' = ty + b*dx + d*dy
```

## cg_matrix_scale

```c
void cg_matrix_scale(struct cg_matrix_t * m, float sx, float sy);
```

post-multiply a scale: `a*=sx; b*=sx; c*=sy; d*=sy`.

## cg_matrix_rotate

```c
void cg_matrix_rotate(struct cg_matrix_t * m, float r);
```

Post-multiply a rotation by `r` radians.

## cg_matrix_shear

```c
void cg_matrix_shear(struct cg_matrix_t * m, float shx, float shy);
```

Post-multiply a shear: `m = m * shear(shx, shy)`.

## cg_matrix_multiply

```c
void cg_matrix_multiply(struct cg_matrix_t * m, struct cg_matrix_t * m1, struct cg_matrix_t * m2);
```

Set `m = m1 * m2`. An optimization path is used when both matrices are axis-aligned (i.e., `b == 0` and `c == 0`).

## cg_matrix_invert

```c
int cg_matrix_invert(struct cg_matrix_t * m);
```

Invert the matrix in place. Returns non-zero on success, `0` if the matrix is singular.

Special cases:
- If `b == 0` and `c == 0` (axis-aligned): returns `0` if `a == 0` or `d == 0`.
- Otherwise: returns `0` if `|det| <= 1e-20`.
