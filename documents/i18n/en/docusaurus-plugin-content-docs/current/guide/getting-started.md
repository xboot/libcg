# Getting Started

This section walks you through a simple example to help you get started with libcg in minutes.

## Prerequisites

1. libcg has been built following the [Build and Install](./build-guide) instructions;
2. A C99-compliant compiler (gcc / clang).

## Step 1: Create a Surface and Context

All drawing operations require a surface (pixel canvas) and a context (drawing context):

```c
#include <cg.h>

struct cg_surface_t * surface = cg_surface_create(256, 256);
struct cg_ctx_t * ctx = cg_create(surface);
```

## Step 2: Draw a Shape

Draw a red filled rectangle:

```c
cg_set_source_rgba(ctx, 1.0, 0.0, 0.0, 1.0);
cg_rectangle(ctx, 50, 50, 156, 156);
cg_fill(ctx);
```

## Step 3: Stroke

Draw a rectangle with a blue outline:

```c
cg_set_source_rgba(ctx, 0.0, 0.0, 1.0, 1.0);
cg_set_line_width(ctx, 3.0);
cg_rectangle(ctx, 50, 50, 156, 156);
cg_stroke(ctx);
```

## Step 4: Free Resources

Once drawing is complete, release the context and surface:

```c
cg_destroy(ctx);
cg_surface_destroy(surface);
```

## Complete Example

```c
#include <cg.h>

int main(void)
{
    struct cg_surface_t * surface = cg_surface_create(256, 256);
    struct cg_ctx_t * ctx = cg_create(surface);

    /* Red fill */
    cg_set_source_rgba(ctx, 1.0, 0.0, 0.0, 1.0);
    cg_rectangle(ctx, 50, 50, 156, 156);
    cg_fill(ctx);

    /* Blue stroke */
    cg_set_source_rgba(ctx, 0.0, 0.0, 1.0, 1.0);
    cg_set_line_width(ctx, 3.0);
    cg_rectangle(ctx, 50, 50, 156, 156);
    cg_stroke(ctx);

    /* Read surface->pixels to obtain the RGBA32 pixel data */
    /* unsigned char * pixels = surface->pixels; */

    cg_destroy(ctx);
    cg_surface_destroy(surface);
    return 0;
}
```

## Further Reading

- [API Reference](../api/architecture): detailed descriptions of all available functions;
- [Usage Examples](../examples/basic-shapes): complete code for various drawing scenarios;
- [Reference List](../reference/enumerations): a quick lookup of enumeration types and default values.
