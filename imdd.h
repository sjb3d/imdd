/*
	imdd: Immediate Mode Debug Draw

	See https://github.com/sjb3d/imdd for usage guide.
*/

#pragma once

#include <stddef.h>
#include <stdint.h>
#include "imdd_atomic.h"
#include "imdd_simd.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	IMDD_STYLE_FILLED,
	IMDD_STYLE_WIRE,
	IMDD_STYLE_COUNT	// keep last
} imdd_style_enum_t;

typedef enum {
	IMDD_SHAPE_LINE,
	IMDD_SHAPE_TRIANGLE,
	IMDD_SHAPE_AABB,
	IMDD_SHAPE_OBB,
	IMDD_SHAPE_SPHERE,
	IMDD_SHAPE_ELLIPSOID,
	IMDD_SHAPE_CONE,
	IMDD_SHAPE_CYLINDER,
	IMDD_SHAPE_COUNT		// keep last
} imdd_shape_enum_t;

typedef enum {
	IMDD_BLEND_OPAQUE,
	IMDD_BLEND_ALPHA,
	IMDD_BLEND_COUNT	// keep last
} imdd_blend_enum_t;

typedef enum {
	IMDD_ZMODE_TEST,
	IMDD_ZMODE_NO_TEST,
	IMDD_ZMODE_COUNT	// keep last
} imdd_zmode_enum_t;

typedef struct imdd_shape_store_tag imdd_shape_store_t;

#define IMDD_APPROX_SHAPE_SIZE_IN_BYTES		64

imdd_shape_store_t *imdd_init(void *mem, uint32_t size);

void imdd_reset(imdd_shape_store_t *store);

void imdd_reserve(
	imdd_shape_store_t *store,
	imdd_shape_enum_t shape,
	imdd_style_enum_t style,
	imdd_zmode_enum_t zmode,
	uint32_t color,
	uint32_t qw_count,
	void **data);

static inline
void IMDD_VECTORCALL imdd_line(
	imdd_shape_store_t *store,
	imdd_zmode_enum_t zmode,
	imdd_v4 start,
	imdd_v4 end,
	uint32_t color)
{
	imdd_v4 *data = NULL;
	imdd_reserve(
		store,
		IMDD_SHAPE_LINE,
		IMDD_STYLE_WIRE,
		zmode,
		color,
		2,
		(void **)&data);
	if (data) {
		data[0] = start;
		data[1] = end;
	}
}

static inline
void IMDD_VECTORCALL imdd_triangle(
	imdd_shape_store_t *store,
	imdd_style_enum_t style,
	imdd_zmode_enum_t zmode,
	imdd_v4 pos_a,
	imdd_v4 pos_b,
	imdd_v4 pos_c,
	uint32_t color)
{
	imdd_v4 *data = NULL;
	imdd_reserve(
		store,
		IMDD_SHAPE_TRIANGLE,
		style,
		zmode,
		color,
		3,
		(void **)&data);
	if (data) {
		data[0] = pos_a;
		data[1] = pos_b;
		data[2] = pos_c;
	}
}

static inline
void IMDD_VECTORCALL imdd_shape(
	imdd_shape_store_t *store,
	imdd_shape_enum_t shape,
	imdd_style_enum_t style,
	imdd_zmode_enum_t zmode,
	imdd_v4 x_axis_vec,
	imdd_v4 y_axis_vec,
	imdd_v4 z_axis_vec,
	imdd_v4 centre,
	uint32_t color)
{
	imdd_v4 *data = NULL;
	imdd_reserve(
		store,
		shape,
		style,
		zmode,
		color,
		3,
		(void **)&data);
	if (data) {
		imdd_v4 const parity = imdd_v4_dot3(imdd_v4_cross(x_axis_vec, y_axis_vec), z_axis_vec);

		imdd_v4 r0 = imdd_v4_mul_sign(x_axis_vec, parity);
		imdd_v4 r1 = y_axis_vec;
		imdd_v4 r2 = z_axis_vec;
		imdd_v4 r3 = centre;
		imdd_v4_transpose_inplace(r0, r1, r2, r3);

		data[0] = r0;
		data[1] = r1;
		data[2] = r2;
	}
}

static inline
void IMDD_VECTORCALL imdd_aabb(
	imdd_shape_store_t *store,
	imdd_style_enum_t style,
	imdd_zmode_enum_t zmode,
	imdd_v4 min,
	imdd_v4 max,
	uint32_t color)
{
	imdd_v4 *data = NULL;
	imdd_reserve(
		store,
		IMDD_SHAPE_AABB,
		style,
		zmode,
		color,
		2,
		(void **)&data);
	if (data) {
		data[0] = min;
		data[1] = max;
	}
}

static inline
void IMDD_VECTORCALL imdd_obb(
	imdd_shape_store_t *store,
	imdd_style_enum_t style,
	imdd_zmode_enum_t zmode,
	imdd_v4 x_axis_vec,
	imdd_v4 y_axis_vec,
	imdd_v4 z_axis_vec,
	imdd_v4 centre,
	uint32_t color)
{
	imdd_shape(store, IMDD_SHAPE_OBB, style, zmode, x_axis_vec, y_axis_vec, z_axis_vec, centre, color);
}

static inline
void IMDD_VECTORCALL imdd_sphere(
	imdd_shape_store_t *store,
	imdd_style_enum_t style,
	imdd_zmode_enum_t zmode,
	imdd_v4 centre_radius,
	uint32_t color)
{
	imdd_v4 *data = NULL;
	imdd_reserve(
		store,
		IMDD_SHAPE_SPHERE,
		style,
		zmode,
		color,
		1,
		(void **)&data);
	if (data) {
		data[0] = centre_radius;
	}
}

static inline
void IMDD_VECTORCALL imdd_ellipsoid(
	imdd_shape_store_t *store,
	imdd_style_enum_t style,
	imdd_zmode_enum_t zmode,
	imdd_v4 x_axis_vec,
	imdd_v4 y_axis_vec,
	imdd_v4 z_axis_vec,
	imdd_v4 centre,
	uint32_t color)
{
	imdd_shape(store, IMDD_SHAPE_ELLIPSOID, style, zmode, x_axis_vec, y_axis_vec, z_axis_vec, centre, color);
}

static inline
void IMDD_VECTORCALL imdd_cone(
	imdd_shape_store_t *store,
	imdd_style_enum_t style,
	imdd_zmode_enum_t zmode,
	imdd_v4 x_axis_vec,
	imdd_v4 y_axis_vec,
	imdd_v4 z_axis_vec,
	imdd_v4 apex,
	uint32_t color)
{
	imdd_shape(store, IMDD_SHAPE_CONE, style, zmode, x_axis_vec, y_axis_vec, z_axis_vec, apex, color);
}

static inline
void IMDD_VECTORCALL imdd_cylinder(
	imdd_shape_store_t *store,
	imdd_style_enum_t style,
	imdd_zmode_enum_t zmode,
	imdd_v4 x_axis_vec,
	imdd_v4 y_axis_vec,
	imdd_v4 z_axis_vec,
	imdd_v4 apex,
	uint32_t color)
{
	imdd_shape(store, IMDD_SHAPE_CYLINDER, style, zmode, x_axis_vec, y_axis_vec, z_axis_vec, apex, color);
}

static inline
void IMDD_VECTORCALL imdd_frustum(
	imdd_shape_store_t *store,
	imdd_style_enum_t style,
	imdd_zmode_enum_t zmode,
	imdd_v4 x_axis_vec,
	imdd_v4 y_axis_vec,
	imdd_v4 z_axis_vec,
	imdd_v4 apex,
	uint32_t color)
{
	imdd_v4 const far_centre = imdd_v4_add(apex, z_axis_vec);

	imdd_v4 const parity = imdd_v4_dot3(imdd_v4_cross(x_axis_vec, y_axis_vec), z_axis_vec);
	imdd_v4 const h_vec = imdd_v4_mul_sign(x_axis_vec, parity);

	imdd_v4 const edge0 = imdd_v4_sub(far_centre, h_vec);
	imdd_v4 const edge1 = imdd_v4_add(far_centre, h_vec);

	imdd_v4 const corner0 = imdd_v4_sub(edge0, y_axis_vec);
	imdd_v4 const corner1 = imdd_v4_sub(edge1, y_axis_vec);
	imdd_v4 const corner2 = imdd_v4_add(edge0, y_axis_vec);
	imdd_v4 const corner3 = imdd_v4_add(edge1, y_axis_vec);

	if (style == IMDD_STYLE_FILLED) {
		imdd_triangle(store, IMDD_STYLE_FILLED, zmode, corner0, apex, corner1, color);
		imdd_triangle(store, IMDD_STYLE_FILLED, zmode, corner1, apex, corner3, color);
		imdd_triangle(store, IMDD_STYLE_FILLED, zmode, corner3, apex, corner2, color);
		imdd_triangle(store, IMDD_STYLE_FILLED, zmode, corner2, apex, corner0, color);
		imdd_triangle(store, IMDD_STYLE_FILLED, zmode, corner0, corner1, corner3, color);
		imdd_triangle(store, IMDD_STYLE_FILLED, zmode, corner3, corner2, corner0, color);
	} else {
		imdd_line(store, zmode, apex, corner0, color);
		imdd_line(store, zmode, apex, corner1, color);
		imdd_line(store, zmode, apex, corner2, color);
		imdd_line(store, zmode, apex, corner3, color);
		imdd_line(store, zmode, corner0, corner1, color);
		imdd_line(store, zmode, corner2, corner3, color);
		imdd_line(store, zmode, corner0, corner2, color);
		imdd_line(store, zmode, corner1, corner3, color);
	}
}

#ifdef IMDD_IMPLEMENTATION

#include "imdd_store.h"

imdd_shape_store_t *imdd_init(void *mem, uint32_t size)
{
	uintptr_t mem_start = (uintptr_t)mem;
	uintptr_t mem_end = mem_start + size;

	// take store from memory
	imdd_shape_store_t *const store = (imdd_shape_store_t *)mem_start;
	mem_start += sizeof(imdd_shape_store_t);

	// use approx 1/8 of the memory for headers
	imdd_shape_header_t *const header_mem = (imdd_shape_header_t *)mem_start;
	uint32_t const header_capacity = (uint32_t)((mem_end - mem_start)/8)/sizeof(imdd_shape_header_t);
	mem_start += header_capacity*sizeof(imdd_shape_header_t);

	// and the rest for data
	uintptr_t const align_mask = sizeof(imdd_v4) - 1;
	mem_start = (mem_start + align_mask) & ~align_mask;
	imdd_v4 *const data_mem = (imdd_v4 *)mem_start;
	uint32_t const data_capacity = (uint32_t)(mem_end - mem_start)/sizeof(imdd_v4);

	// write the store out
	store->header_store = header_mem;
	imdd_atomic_store(&store->header_count, 0);
	store->header_capacity = header_capacity;
	for (uint32_t i = 0; i < IMDD_SHAPE_BUCKET_COUNT; ++i) {
		imdd_atomic_store(&store->bucket_sizes[i], 0);
	}
	store->data_qw_store = data_mem;
	imdd_atomic_store(&store->data_qw_count, 0);
	store->data_qw_capacity = data_capacity;
	return store;
}

void imdd_reset(imdd_shape_store_t *store)
{
	// empty the store
	imdd_atomic_store(&store->header_count, 0);
	for (uint32_t i = 0; i < IMDD_SHAPE_BUCKET_COUNT; ++i) {
		imdd_atomic_store(&store->bucket_sizes[i], 0);
	}
	imdd_atomic_store(&store->data_qw_count, 0);
}

void imdd_reserve(
	imdd_shape_store_t *store,
	imdd_shape_enum_t shape,
	imdd_style_enum_t style,
	imdd_zmode_enum_t zmode,
	uint32_t color,
	uint32_t data_qw_count,
	void **data)
{
	// check for a valid store
	if (!store) {
		return;
	}

	// check for space
	uint32_t const header_offset = imdd_atomic_fetch_add(&store->header_count, 1);
	if (header_offset >= store->header_capacity) {
		return;
	}

	uint32_t const data_qw_offset = imdd_atomic_fetch_add(&store->data_qw_count, data_qw_count);
	if (data_qw_offset >= store->data_qw_capacity) {
		// write bad shape to header to ensure it is skipped later
		shape = IMDD_SHAPE_COUNT;
	} else {
		// valid data area
		*data = store->data_qw_store + data_qw_offset;
	}

	// write the header, make space for data (filled in by caller)
	imdd_shape_header_t header;
	header.style = style;
	header.zmode = zmode;
	header.blend = ((color >> 24) != 0xff) ? IMDD_BLEND_ALPHA : IMDD_BLEND_OPAQUE;
	header.shape = shape;
	header.data_qw_offset = data_qw_offset;
	header.color = color;
	store->header_store[header_offset] = header;

	// account for an instance of this shape in buckets
	if (shape < IMDD_SHAPE_COUNT) {
		uint32_t const bucket_index = imdd_bucket_index_from_shape_header(header);
		imdd_atomic_fetch_add(&store->bucket_sizes[bucket_index], 1);
	}
}

#endif // def IMDD_IMPLEMENTATION

#ifdef __cplusplus
} // extern "C"
#endif
