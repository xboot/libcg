# Build and Install

This document describes how to build the libcg library and the example programs from source.

## Dependencies

libcg depends only on the standard C library (libc and libm); no additional third-party libraries need to be installed. Building the example programs requires a C99-compliant compiler (such as gcc or clang).

## Build the Library and Examples

Run the following from the source root directory:

```shell
cd libcg
make
```

After the build completes, the following will be generated:

- `src/libcg.a` — the static library
- `examples/examples` — the example program (which produces PNG images in the examples directory when run)

## Build Only the Library

```shell
make -C src
```

This produces the `src/libcg.a` static library along with the `src/cg.o` and `src/xft.o` object files.

## Build Only the Examples

```shell
make -C examples
```

The example program links against `src/libcg.a`. When run, it outputs several PNG images to the current directory, demonstrating the various drawing capabilities of libcg.

## Cross Compilation

Specify the cross-compilation toolchain prefix via the `CROSS_COMPILE` variable:

```shell
make CROSS_COMPILE=arm-linux-gnueabihf-
```

## Integrating into a Project

Copy the following files into your project to start using libcg:

- `src/cg.h` — main header file
- `src/cg.c` — implementation file
- `src/xft.h` — FreeType outline processing header
- `src/xft.c` — FreeType outline processing implementation

Include the header in your source and link against the math library:

```c
#include <cg.h>
```

Link with `-lm` when compiling:

```shell
gcc -I src my_program.c src/cg.c src/xft.c -lm -o my_program
```

## Cleaning

```shell
make clean
```

Removes all generated object files, the static library, and the example program.
