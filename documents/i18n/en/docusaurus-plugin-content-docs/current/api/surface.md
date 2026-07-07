# Surface Functions

A surface is the pixel canvas that stores pixel data in premultiplied RGBA32 format. All drawing operations are ultimately rendered onto a surface.

## cg_surface_create

```c
struct cg_surface_t * cg_surface_create(int width, int height);
```

Allocates a new surface of the given size. The pixel buffer is zero-initialized (fully transparent black). `owndata` is set to `1`.

## cg_surface_create_for_data

```c
struct cg_surface_t * cg_surface_create_for_data(int width, int height, void * pixels);
```

Creates a surface wrapping an existing pixel buffer. `pixels` must be at least `width * height * 4` bytes in premultiplied RGBA32 format. `owndata` is set to `0`, meaning calling `cg_surface_destroy` will **not** free the pixel data — the caller is responsible for its lifetime.

## cg_surface_destroy

```c
void cg_surface_destroy(struct cg_surface_t * surface);
```

Decrements the reference count. When it reaches zero, frees the pixel buffer (if `owndata == 1`) and the surface struct. Safe to call with `NULL`.

## cg_surface_reference

```c
struct cg_surface_t * cg_surface_reference(struct cg_surface_t * surface);
```

Increments the reference count and returns the surface. Safe to call with `NULL` (returns `NULL`).

## Pixel Format

Surface pixels are stored in **premultiplied RGBA32** format (row-major), with 4 bytes per pixel:

```
[Offset]  Byte      Meaning
0         R'        Premultiplied red component
1         G'        Premultiplied green component
2         B'        Premultiplied blue component
3         A         Alpha transparency
```

Where `R' = R * A / 255`, `G' = G * A / 255`, `B' = B * A / 255`.

`stride` equals `width * 4` and can be accessed via `surface->stride`. The pixel data pointer is accessed via `surface->pixels`.

## Usage Example

```c
/* Create a 256x256 surface */
struct cg_surface_t * surface = cg_surface_create(256, 256);

/* Drawing operations... */

/* Access pixel data */
unsigned char * pixels = surface->pixels;
int width = surface->width;
int height = surface->height;
int stride = surface->stride;

/* Release */
cg_surface_destroy(surface);
```
