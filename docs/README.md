# imdd

**I**mmediate **M**ode **D**ebug **D**raw library written in C.

The library provides a simple immediate-mode interface to draw 3D shapes, to make it easy to inspect or debug state in games or other 3D applications.
Utility functions are provided for rendering, with drop-in implementations for OpenGL 3.2 and Vulkan.

## Usage

The library is header-only and compiles as C or C++.  Include the header `imdd.h` in your project to use the library.  Use a built-in renderer by including that header, or implement your own.

```C
// one source file should define this to generate the implementation
#define IMDD_IMPLEMENTATION
#include "imdd.h"
#include "imdd_draw_gl3.h"

// create a shape store at startup
uint32_t shape_mem_size = IMDD_APPROX_SHAPE_SIZE_IN_BYTES*shape_count;
imdd_shape_store_t *store = imdd_init(malloc(shape_mem_size), shape_mem_size);

// create a renderer at startup
imdd_gl3_context_t ctx;
imdd_gl3_init(&ctx, shape_count, shape_count, shape_count);

// game frame loop
for (;;) {
	// clear existing shapes at the start of the frame
	imdd_reset(store);

	// ...

	// emit shapes at any point during the frame (from multiple threads)
	imdd_aabb(store, IMDD_STYLE_FILLED, IMDD_ZMODE_TEST, aabb_min, aabb_max, 0xff0000ffU);
	if (show_sphere) {
		imdd_sphere(store, IMDD_STYLE_WIRE, IMDD_ZMODE_TEST, centre_radius, 0xff00ff7fU);
	}

	// ...

	// render the shapes at the end of the frame
	imdd_gl3_update(&ctx, &store, 1);
	imdd_gl3_draw(&ctx, proj_from_world);

	// ...
}
```

## Examples

See `example_gl3.c` or `example_vulkan.c` for a test scene that uses all shapes:

![example](https://raw.githubusercontent.com/sjb3d/imdd/master/docs/example.png)

## Details

The library is intended to solve two problems:

- Efficient emission of large numbers of shapes from multiple threads simultaneously
- Batching of these shapes into arrays of vertices or instance transforms for efficient rendering

### Emitting Shapes

Emitting shapes is handled by the core library in `imdd.h`, which appends shapes into a *store* in memory:

- The following shapes are supported:
  - Line, Triangle, Cube, Sphere, Cone, Cylinder
- Each call to emit a shape reserves some space and writes out the parameters to memory
  - Parameters are written using SIMD instructions from `imdd_simd.h`, some parameters are expected to be passed in as SIMD types
  - Reserving space is done atomically using `imdd_atomics.h` to support multiple threads
- All shapes except line can be drawn filled or wireframe
- All shapes can be drawn with or without Z test

### Rendering Shapes

Utility functions to simplify implementing a renderer can be found in `imdd_draw_utils.h`:

- Functions to generate static vertex and index buffers for all shapes (filled and wireframe)
- Functions to transform one or many *stores* into batches of vertices or transforms:
  - Lines and triangles generate vertex arrays for drawing directly
  - All other shapes generate arrays of transforms for instanced drawing
  - Arrays are partitioned into batches so that each combination of z test, blend mode and mesh can be drawn separately

By using the code from `imdd_draw_utils.h`, a renderer typically just has to:

- Provide shaders for instanced and non-instanced rendering
- Manage vertex buffer memory for static meshes and dynamic vertex and transform arrays
- Set render state and emit a draw call for each batch

Renderer implementations are provided for:

API | Header | Notes
--- | --- | ---
OpenGL 3.2 | `imdd_draw_gl3.h` | Currently requires the `GL_ARB_base_instance` extension for `glDrawElementsInstancedBaseInstance`.
Vulkan | `imdd_draw_vulkan.h` | Supports the `OVR_multiview2` extension for stereo rendering (tested on Oculus Quest).

## License

The library is released under the MIT license.
