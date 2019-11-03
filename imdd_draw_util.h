#pragma once

#include "imdd.h"
#include "imdd_store.h"
#include <math.h>
#include <string.h> // for memcpy

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	IMDD_MESH_BOX,
	IMDD_MESH_SPHERE,
	IMDD_MESH_CONE,
	IMDD_MESH_CYLINDER,
	IMDD_MESH_COUNT
} imdd_mesh_enum_t;

static imdd_mesh_enum_t const g_imdd_mesh_from_shape[IMDD_SHAPE_COUNT] = {
	IMDD_MESH_COUNT,	// IMDD_SHAPE_LINE
	IMDD_MESH_COUNT,	// IMDD_SHAPE_TRIANGLE
	IMDD_MESH_BOX,		// IMDD_SHAPE_AABB
	IMDD_MESH_BOX,		// IMDD_SHAPE_OBB
	IMDD_MESH_SPHERE,	// IMDD_SHAPE_SPHERE
	IMDD_MESH_SPHERE,	// IMDD_SHAPE_ELLIPSOID
	IMDD_MESH_CONE,		// IMDD_SHAPE_CONE
	IMDD_MESH_CYLINDER	// IMDD_SHAPE_CYLINDER
};

typedef struct {
	float pos[3];
	float normal[3];
} imdd_mesh_filled_vertex_t;

typedef struct {
	float pos[3];
} imdd_mesh_wire_vertex_t;

#define IMDD_PI								3.1415926535f

#define IMDD_FILLED_BOX_VERTEX_COUNT		(4*6)
#define IMDD_FILLED_BOX_INDEX_COUNT			(6*6)

#define IMDD_FILLED_SPHERE_SUB				6
#define IMDD_FILLED_SPHERE_VERTEX_COUNT		(6*(1 + IMDD_FILLED_SPHERE_SUB)*(1 + IMDD_FILLED_SPHERE_SUB))
#define IMDD_FILLED_SPHERE_INDEX_COUNT		(6*6*IMDD_FILLED_SPHERE_SUB*IMDD_FILLED_SPHERE_SUB)

#define IMDD_FILLED_CONE_SEGMENT_COUNT		18
#define IMDD_FILLED_CONE_VERTEX_COUNT		(3*IMDD_FILLED_CONE_SEGMENT_COUNT + 1)
#define IMDD_FILLED_CONE_INDEX_COUNT		(9*IMDD_FILLED_CONE_SEGMENT_COUNT)

#define IMDD_FILLED_CYLINDER_SEGMENT_COUNT	18
#define IMDD_FILLED_CYLINDER_VERTEX_COUNT	(4*IMDD_FILLED_CYLINDER_SEGMENT_COUNT + 2)
#define IMDD_FILLED_CYLINDER_INDEX_COUNT	(12*IMDD_FILLED_CYLINDER_SEGMENT_COUNT)

#define IMDD_WIRE_BOX_VERTEX_COUNT			(8)
#define IMDD_WIRE_BOX_INDEX_COUNT			(2*12)

#define IMDD_WIRE_SPHERE_SUB				6
#define IMDD_WIRE_SPHERE_VERTEX_COUNT		(6*(1 + IMDD_WIRE_SPHERE_SUB)*(1 + IMDD_WIRE_SPHERE_SUB))
#define IMDD_WIRE_SPHERE_INDEX_COUNT		(6*4*(1 + IMDD_WIRE_SPHERE_SUB)*IMDD_WIRE_SPHERE_SUB)

#define IMDD_WIRE_CONE_SEGMENT_COUNT		18
#define IMDD_WIRE_CONE_VERTEX_COUNT			(IMDD_WIRE_CONE_SEGMENT_COUNT + 1)
#define IMDD_WIRE_CONE_INDEX_COUNT			(4*IMDD_WIRE_CONE_SEGMENT_COUNT)

#define IMDD_WIRE_CYLINDER_SEGMENT_COUNT	18
#define IMDD_WIRE_CYLINDER_VERTEX_COUNT		(2*IMDD_WIRE_CYLINDER_SEGMENT_COUNT)
#define IMDD_WIRE_CYLINDER_INDEX_COUNT		(6*IMDD_WIRE_CYLINDER_SEGMENT_COUNT)

static
void imdd_write_filled_box(void *vertex_base, uint32_t vertex_offset, uint16_t *indices)
{
	imdd_mesh_filled_vertex_t *vertices = (imdd_mesh_filled_vertex_t *)vertex_base + vertex_offset;
	for (uint32_t face = 0; face < 6; ++face) {
		// make normal and tangent
		float n[3] = { 0.f, 0.f, 0.f };
		float t[3] = { 0.f, 0.f ,0.f };
		n[face/2] = (face & 1) ? 1.f : -1.f;
		t[(1 + face/2) % 3] = 1.f;

		// make tangent space
		imdd_v4 const normal = imdd_v4_load_3f(n);
		imdd_v4 const tangent = imdd_v4_load_3f(t);
		imdd_v4 const bitangent = imdd_v4_cross(normal, tangent);

		// write quad
		imdd_v4_store_3f(vertices[0].pos, imdd_v4_sub(imdd_v4_sub(normal, tangent), bitangent));
		imdd_v4_store_3f(vertices[0].normal, normal);
		imdd_v4_store_3f(vertices[1].pos, imdd_v4_sub(imdd_v4_add(normal, tangent), bitangent));
		imdd_v4_store_3f(vertices[1].normal, normal);
		imdd_v4_store_3f(vertices[2].pos, imdd_v4_add(imdd_v4_sub(normal, tangent), bitangent));
		imdd_v4_store_3f(vertices[2].normal, normal);
		imdd_v4_store_3f(vertices[3].pos, imdd_v4_add(imdd_v4_add(normal, tangent), bitangent));
		imdd_v4_store_3f(vertices[3].normal, normal);
		vertices += 4;

		// write triangles
		uint32_t const i = vertex_offset + 4*face;
		indices[0] = (uint16_t)(i + 0);
		indices[1] = (uint16_t)(i + 1);
		indices[2] = (uint16_t)(i + 2);
		indices[3] = (uint16_t)(i + 3);
		indices[4] = (uint16_t)(i + 2);
		indices[5] = (uint16_t)(i + 1);
		indices += 6;
	}
}

static
void imdd_write_filled_sphere(void *vertex_base, uint32_t vertex_offset, uint16_t *indices)
{
	imdd_mesh_filled_vertex_t *vertices = (imdd_mesh_filled_vertex_t *)vertex_base + vertex_offset;
	for (uint32_t face = 0; face < 6; ++face) {
		// make normal and tangent
		float n[3] = { 0.f, 0.f, 0.f };
		float t[3] = { 0.f, 0.f ,0.f };
		n[face/2] = (face & 1) ? 1.f : -1.f;
		t[(1 + face/2) % 3] = 1.f;

		// make tangent space
		imdd_v4 const normal = imdd_v4_load_3f(n);
		imdd_v4 const tangent = imdd_v4_load_3f(t);
		imdd_v4 const bitangent = imdd_v4_cross(normal, tangent);

		// tessellate the quad
		for (uint32_t y = 0; y <= IMDD_FILLED_SPHERE_SUB; ++y)
		for (uint32_t x = 0; x <= IMDD_FILLED_SPHERE_SUB; ++x) {
			float const tx = 2.f*(float)x/(float)IMDD_FILLED_SPHERE_SUB - 1.f;
			float const ty = 2.f*(float)y/(float)IMDD_FILLED_SPHERE_SUB - 1.f;

			imdd_v4 const offset = imdd_v4_add(
				imdd_v4_mul(tangent, imdd_v4_init_1f(tx)),
				imdd_v4_mul(bitangent, imdd_v4_init_1f(ty)));
			imdd_v4 const dir = imdd_v4_normalize3(imdd_v4_add(normal, offset));

			imdd_v4_store_3f(vertices[0].pos, dir);
			imdd_v4_store_3f(vertices[0].normal, dir);
			++vertices;
		}

		// write triangles
		uint32_t const face_vertex_offset = vertex_offset + face*((1 + IMDD_FILLED_SPHERE_SUB)*(1 + IMDD_FILLED_SPHERE_SUB));
		for (uint32_t y = 0; y < IMDD_FILLED_SPHERE_SUB; ++y)
		for (uint32_t x = 0; x < IMDD_FILLED_SPHERE_SUB; ++x) {
			uint32_t const i0 = face_vertex_offset + y*(1 + IMDD_FILLED_SPHERE_SUB) + x;
			uint32_t const i1 = i0 + 1;
			uint32_t const i2 = i0 + (1 + IMDD_FILLED_SPHERE_SUB);
			uint32_t const i3 = i2 + 1;
			indices[0] = (uint16_t)i0;
			indices[1] = (uint16_t)i1;
			indices[2] = (uint16_t)i2;
			indices[3] = (uint16_t)i3;
			indices[4] = (uint16_t)i2;
			indices[5] = (uint16_t)i1;
			indices += 6;
		}
	}
}

static
void imdd_write_filled_cone(void *vertex_base, uint32_t vertex_offset, uint16_t *indices)
{
	imdd_mesh_filled_vertex_t *vertices = (imdd_mesh_filled_vertex_t *)vertex_base + vertex_offset;
	float const sqrt_half = sqrtf(1.f/2.f);

	// quads around apex
	for (uint32_t i = 0; i < IMDD_FILLED_CONE_SEGMENT_COUNT; ++i) {
		float const phi = 2.f*IMDD_PI*(float)i/(float)IMDD_FILLED_CONE_SEGMENT_COUNT;
		float const cos_phi = cosf(phi);
		float const sin_phi = sinf(phi);
		vertices[0].pos[0] = cos_phi;
		vertices[0].pos[1] = sin_phi;
		vertices[0].pos[2] = 1.f;
		vertices[0].normal[0] = cos_phi*sqrt_half;
		vertices[0].normal[1] = sin_phi*sqrt_half;
		vertices[0].normal[2] = -sqrt_half;
		vertices[1].pos[0] = 0.f;
		vertices[1].pos[1] = 0.f;
		vertices[1].pos[2] = 0.f;
		vertices[1].normal[0] = cos_phi*sqrt_half;
		vertices[1].normal[1] = sin_phi*sqrt_half;
		vertices[1].normal[2] = -sqrt_half;
		vertices += 2;
	}
	for (uint32_t i = 0; i < IMDD_FILLED_CONE_SEGMENT_COUNT; ++i) {
		uint32_t const i0 = vertex_offset + 2*i;
		uint32_t const i1 = i0 + 1;
		uint32_t const i2 = vertex_offset + 2*((i + 1) % IMDD_FILLED_CONE_SEGMENT_COUNT);
		uint32_t const i3 = i2 + 1;
		indices[0] = (uint16_t)i0;
		indices[1] = (uint16_t)i1;
		indices[2] = (uint16_t)i2;
		indices[3] = (uint16_t)i3;
		indices[4] = (uint16_t)i2;
		indices[5] = (uint16_t)i1;
		indices += 6;
	}
	vertex_offset += 2*IMDD_FILLED_CONE_SEGMENT_COUNT;

	// end cap
	for (uint32_t i = 0; i < IMDD_FILLED_CONE_SEGMENT_COUNT; ++i) {
		float const phi = 2.f*IMDD_PI*(float)i/(float)IMDD_FILLED_CONE_SEGMENT_COUNT;
		float const cos_phi = cosf(phi);
		float const sin_phi = sinf(phi);
		vertices[0].pos[0] = cos_phi;
		vertices[0].pos[1] = sin_phi;
		vertices[0].pos[2] = 1.f;
		vertices[0].normal[0] = 0.f;
		vertices[0].normal[1] = 0.f;
		vertices[0].normal[2] = 1.f;
		++vertices;
	}
	vertices[0].pos[0] = 0.f;
	vertices[0].pos[1] = 0.f;
	vertices[0].pos[2] = 1.f;
	vertices[0].normal[0] = 0.f;
	vertices[0].normal[1] = 0.f;
	vertices[0].normal[2] = 1.f;
	++vertices;
	for (uint32_t i = 0; i < IMDD_FILLED_CONE_SEGMENT_COUNT; ++i) {
		uint32_t const i0 = vertex_offset + i;
		uint32_t const i1 = vertex_offset + ((i + 1) % IMDD_FILLED_CONE_SEGMENT_COUNT);
		uint32_t const i2 = vertex_offset + IMDD_FILLED_CONE_SEGMENT_COUNT;
		indices[0] = (uint16_t)i0;
		indices[1] = (uint16_t)i1;
		indices[2] = (uint16_t)i2;
		indices += 3;
	}
}

static
void imdd_write_filled_cylinder(void *vertex_base, uint32_t vertex_offset, uint16_t *indices)
{
	imdd_mesh_filled_vertex_t *vertices = (imdd_mesh_filled_vertex_t *)vertex_base + vertex_offset;

	// quads around
	for (uint32_t i = 0; i < IMDD_FILLED_CYLINDER_SEGMENT_COUNT; ++i) {
		float const phi = 2.f*IMDD_PI*(float)i/(float)IMDD_FILLED_CYLINDER_SEGMENT_COUNT;
		float const cos_phi = cosf(phi);
		float const sin_phi = sinf(phi);
		vertices[0].pos[0] = cos_phi;
		vertices[0].pos[1] = sin_phi;
		vertices[0].pos[2] = -1.f;
		vertices[0].normal[0] = cos_phi;
		vertices[0].normal[1] = sin_phi;
		vertices[0].normal[2] = 0.f;
		vertices[1].pos[0] = cos_phi;
		vertices[1].pos[1] = sin_phi;
		vertices[1].pos[2] = 1.f;
		vertices[1].normal[0] = cos_phi;
		vertices[1].normal[1] = sin_phi;
		vertices[1].normal[2] = 0.f;
		vertices += 2;
	}
	for (uint32_t i = 0; i < IMDD_FILLED_CYLINDER_SEGMENT_COUNT; ++i) {
		uint32_t const i0 = vertex_offset + 2*i;
		uint32_t const i1 = i0 + 1;
		uint32_t const i2 = vertex_offset + 2*((i + 1) % IMDD_FILLED_CYLINDER_SEGMENT_COUNT);
		uint32_t const i3 = i2 + 1;
		indices[0] = (uint16_t)i0;
		indices[1] = (uint16_t)i2;
		indices[2] = (uint16_t)i1;
		indices[3] = (uint16_t)i1;
		indices[4] = (uint16_t)i2;
		indices[5] = (uint16_t)i3;
		indices += 6;
	}
	vertex_offset += 2*IMDD_FILLED_CYLINDER_SEGMENT_COUNT;

	// end caps
	for (uint32_t k = 0; k < 2; ++k) {
		float const nz = (k != 0) ? 1.f : -1.f;
		for (uint32_t i = 0; i < IMDD_FILLED_CYLINDER_SEGMENT_COUNT; ++i) {
			float const phi = 2.f*IMDD_PI*(float)i/(float)IMDD_FILLED_CYLINDER_SEGMENT_COUNT;
			float const cos_phi = cosf(phi);
			float const sin_phi = sinf(phi);
			vertices[0].pos[0] = cos_phi;
			vertices[0].pos[1] = sin_phi;
			vertices[0].pos[2] = nz;
			vertices[0].normal[0] = 0.f;
			vertices[0].normal[1] = 0.f;
			vertices[0].normal[2] = nz;
			++vertices;
		}
		vertices[0].pos[0] = 0.f;
		vertices[0].pos[1] = 0.f;
		vertices[0].pos[2] = nz;
		vertices[0].normal[0] = 0.f;
		vertices[0].normal[1] = 0.f;
		vertices[0].normal[2] = nz;
		++vertices;

		for (uint32_t i = 0; i < IMDD_FILLED_CYLINDER_SEGMENT_COUNT; ++i) {
			uint32_t const i0 = vertex_offset + i;
			uint32_t const i1 = vertex_offset + ((i + 1) % IMDD_FILLED_CYLINDER_SEGMENT_COUNT);
			uint32_t const i2 = vertex_offset + IMDD_FILLED_CYLINDER_SEGMENT_COUNT;
			indices[0] = (uint16_t)((k != 0) ? i0 : i1);
			indices[1] = (uint16_t)((k != 0) ? i1 : i0);
			indices[2] = (uint16_t)i2;
			indices += 3;
		}
		vertex_offset += IMDD_FILLED_CYLINDER_SEGMENT_COUNT + 1;
	}
}

static
void imdd_write_wire_box(void *vertex_base, uint32_t vertex_offset, uint16_t *indices)
{
	imdd_mesh_wire_vertex_t *const vertices = (imdd_mesh_wire_vertex_t *)vertex_base + vertex_offset;
	for (uint32_t i = 0; i < IMDD_WIRE_BOX_VERTEX_COUNT; ++i) {
		vertices[i].pos[0] = (i & 1) ? 1.f : -1.f;
		vertices[i].pos[1] = (i & 2) ? 1.f : -1.f;
		vertices[i].pos[2] = (i & 4) ? 1.f : -1.f;
	}

	static uint8_t const idata[IMDD_WIRE_BOX_INDEX_COUNT] = {
		0, 1,
		2, 3,
		4, 5,
		6, 7,

		0, 2,
		1, 3,
		4, 6,
		5, 7,

		0, 4,
		1, 5,
		2, 6,
		3, 7
	};

	for (uint32_t i = 0; i < IMDD_WIRE_BOX_INDEX_COUNT; ++i) {
		indices[i] = (uint16_t)(vertex_offset + idata[i]);
	}
}

static
void imdd_write_wire_sphere(void *vertex_base, uint32_t vertex_offset, uint16_t *indices)
{
	imdd_mesh_wire_vertex_t *vertices = (imdd_mesh_wire_vertex_t *)vertex_base + vertex_offset;
	for (uint32_t face = 0; face < 6; ++face) {
		// make normal and tangent
		float n[3] = { 0.f, 0.f, 0.f };
		float t[3] = { 0.f, 0.f ,0.f };
		n[face/2] = (face & 1) ? 1.f : -1.f;
		t[(1 + face/2) % 3] = 1.f;

		// make tangent space
		imdd_v4 const normal = imdd_v4_load_3f(n);
		imdd_v4 const tangent = imdd_v4_load_3f(t);
		imdd_v4 const bitangent = imdd_v4_cross(normal, tangent);

		// tessellate the quad
		for (uint32_t y = 0; y <= IMDD_WIRE_SPHERE_SUB; ++y)
		for (uint32_t x = 0; x <= IMDD_WIRE_SPHERE_SUB; ++x) {
			float const tx = 2.f*(float)x/(float)IMDD_WIRE_SPHERE_SUB - 1.f;
			float const ty = 2.f*(float)y/(float)IMDD_WIRE_SPHERE_SUB - 1.f;

			imdd_v4 const offset = imdd_v4_add(
				imdd_v4_mul(tangent, imdd_v4_init_1f(tx)),
				imdd_v4_mul(bitangent, imdd_v4_init_1f(ty)));
			imdd_v4 const dir = imdd_v4_normalize3(imdd_v4_add(normal, offset));

			imdd_v4_store_3f(vertices[0].pos, dir);
			++vertices;
		}

		// write lines
		uint32_t const face_vertex_offset = vertex_offset + face*((1 + IMDD_WIRE_SPHERE_SUB)*(1 + IMDD_WIRE_SPHERE_SUB));
		for (uint32_t y = 0; y <= IMDD_WIRE_SPHERE_SUB; ++y)
		for (uint32_t x = 0; x <= IMDD_WIRE_SPHERE_SUB; ++x) {
			uint32_t const i0 = face_vertex_offset + y*(1 + IMDD_WIRE_SPHERE_SUB) + x;
			if (x < IMDD_WIRE_SPHERE_SUB) {
				uint32_t const i1 = i0 + 1;
				indices[0] = (uint16_t)i0;
				indices[1] = (uint16_t)i1;
				indices += 2;
			}
			if (y < IMDD_WIRE_SPHERE_SUB) {
				uint32_t const i1 = i0 + (1 + IMDD_WIRE_SPHERE_SUB);
				indices[0] = (uint16_t)i0;
				indices[1] = (uint16_t)i1;
				indices += 2;
			}
		}
	}
}

static
void imdd_write_wire_cone(void *vertex_base, uint32_t vertex_offset, uint16_t *indices)
{
	imdd_mesh_wire_vertex_t *vertices = (imdd_mesh_wire_vertex_t *)vertex_base + vertex_offset;
	for (uint32_t i = 0; i < IMDD_WIRE_CONE_SEGMENT_COUNT; ++i) {
		float const phi = 2.f*IMDD_PI*(float)i/(float)IMDD_WIRE_CONE_SEGMENT_COUNT;
		float const cos_phi = cosf(phi);
		float const sin_phi = sinf(phi);
		vertices[0].pos[0] = cos_phi;
		vertices[0].pos[1] = sin_phi;
		vertices[0].pos[2] = 1.f;
		++vertices;
	}
	vertices[0].pos[0] = 0.f;
	vertices[0].pos[1] = 0.f;
	vertices[0].pos[2] = 0.f;
	++vertices;

	for (uint32_t i = 0; i < IMDD_WIRE_CONE_SEGMENT_COUNT; ++i) {
		uint32_t const i0 = vertex_offset + i;
		uint32_t const i1 = vertex_offset + ((i + 1) % IMDD_WIRE_CONE_SEGMENT_COUNT);
		uint32_t const i2 = vertex_offset + IMDD_WIRE_CONE_SEGMENT_COUNT;
		indices[0] = (uint16_t)i0;
		indices[1] = (uint16_t)i1;
		indices[2] = (uint16_t)i0;
		indices[3] = (uint16_t)i2;
		indices += 4;
	}
}

static
void imdd_write_wire_cylinder(void *vertex_base, uint32_t vertex_offset, uint16_t *indices)
{
	imdd_mesh_wire_vertex_t *vertices = (imdd_mesh_wire_vertex_t *)vertex_base + vertex_offset;
	for (uint32_t i = 0; i < IMDD_WIRE_CYLINDER_SEGMENT_COUNT; ++i) {
		float const phi = 2.f*IMDD_PI*(float)i/(float)IMDD_WIRE_CYLINDER_SEGMENT_COUNT;
		float const cos_phi = cosf(phi);
		float const sin_phi = sinf(phi);
		vertices[0].pos[0] = cos_phi;
		vertices[0].pos[1] = sin_phi;
		vertices[0].pos[2] = -1.f;
		vertices[1].pos[0] = cos_phi;
		vertices[1].pos[1] = sin_phi;
		vertices[1].pos[2] = 1.f;
		vertices += 2;
	}

	for (uint32_t i = 0; i < IMDD_WIRE_CYLINDER_SEGMENT_COUNT; ++i) {
		uint32_t const i0 = vertex_offset + 2*i;
		uint32_t const i1 = i0 + 1;
		uint32_t const i2 = vertex_offset + 2*((i + 1) % IMDD_WIRE_CYLINDER_SEGMENT_COUNT);
		uint32_t const i3 = i2 + 1;
		indices[1] = (uint16_t)i2;
		indices[0] = (uint16_t)i0;
		indices[2] = (uint16_t)i0;
		indices[3] = (uint16_t)i1;
		indices[4] = (uint16_t)i1;
		indices[5] = (uint16_t)i3;
		indices += 6;
	}
}

typedef void (* imdd_write_mesh_func_t)(void *, uint32_t, uint16_t *);

typedef struct {
	imdd_write_mesh_func_t write_mesh_func;
	uint32_t vertex_count;
	uint32_t index_count;
} imdd_mesh_desc_t;

static imdd_mesh_desc_t const g_imdd_mesh_desc[IMDD_STYLE_COUNT][IMDD_MESH_COUNT] = {
	// IMDD_STYLE_FILLED
	{
		{ &imdd_write_filled_box,		IMDD_FILLED_BOX_VERTEX_COUNT,		IMDD_FILLED_BOX_INDEX_COUNT },
		{ &imdd_write_filled_sphere,	IMDD_FILLED_SPHERE_VERTEX_COUNT,	IMDD_FILLED_SPHERE_INDEX_COUNT },
		{ &imdd_write_filled_cone,		IMDD_FILLED_CONE_VERTEX_COUNT,		IMDD_FILLED_CONE_INDEX_COUNT },
		{ &imdd_write_filled_cylinder,	IMDD_FILLED_CYLINDER_VERTEX_COUNT,	IMDD_FILLED_CYLINDER_INDEX_COUNT }
	},
	// IMDD_STYLE_WIRE
	{
		{ &imdd_write_wire_box,			IMDD_WIRE_BOX_VERTEX_COUNT,			IMDD_WIRE_BOX_INDEX_COUNT },
		{ &imdd_write_wire_sphere,		IMDD_WIRE_SPHERE_VERTEX_COUNT,		IMDD_WIRE_SPHERE_INDEX_COUNT },
		{ &imdd_write_wire_cone,		IMDD_WIRE_CONE_VERTEX_COUNT,		IMDD_WIRE_CONE_INDEX_COUNT },
		{ &imdd_write_wire_cylinder,	IMDD_WIRE_CYLINDER_VERTEX_COUNT,	IMDD_WIRE_CYLINDER_INDEX_COUNT }
	}
};

typedef struct {
	uint32_t vertex_offset;
	uint32_t index_offset;
} imdd_mesh_offsets_t;

typedef struct {
	imdd_mesh_desc_t const *mesh_desc;
	imdd_mesh_offsets_t mesh_offsets[IMDD_MESH_COUNT];
	uint32_t vertex_count;
	uint32_t index_count;
} imdd_mesh_layout_t;

static
void imdd_mesh_layout_init(imdd_mesh_layout_t *mesh_layout, imdd_style_enum_t style)
{
	mesh_layout->mesh_desc = g_imdd_mesh_desc[style];

	uint32_t vertex_count = 0;
	uint32_t index_count = 0;

	for (uint32_t i = 0; i < IMDD_MESH_COUNT; ++i) {
		imdd_mesh_desc_t const *const mesh_desc = mesh_layout->mesh_desc + i;
		imdd_mesh_offsets_t *const mesh_offsets = mesh_layout->mesh_offsets + i;

		mesh_offsets->vertex_offset = vertex_count;
		mesh_offsets->index_offset = index_count;

		vertex_count += mesh_desc->vertex_count;
		index_count += mesh_desc->index_count;
	}
	mesh_layout->vertex_count = vertex_count;
	mesh_layout->index_count = index_count;
}

static
void imdd_mesh_layout_write(
	imdd_mesh_layout_t const *mesh_layout,
	void *vertices,
	uint16_t *indices)
{
	for (uint32_t i = 0; i < IMDD_MESH_COUNT; ++i) {
		imdd_mesh_offsets_t const *const mesh_offsets = mesh_layout->mesh_offsets + i;
		imdd_mesh_desc_t const *const mesh_desc = mesh_layout->mesh_desc + i;

		if (mesh_desc->write_mesh_func) {
			mesh_desc->write_mesh_func(
				vertices,
				mesh_offsets->vertex_offset,
				indices + mesh_offsets->index_offset);
		}
	}
}

/* -------------------------------------------------------------------------
	Internal API for generating transforms and vertices for shapes.
   ------------------------------------------------------------------------- */

typedef struct {
	imdd_v4 row0;
	imdd_v4 row1;
	imdd_v4 row2;
} imdd_instance_transform_t;

typedef struct {
	uint32_t col;
} imdd_instance_color_t;

typedef struct {
	imdd_v4 pos_col;
	imdd_v4 normal_pad;
} imdd_array_filled_vertex_t;

typedef struct {
	imdd_v4 pos_col;
} imdd_array_wire_vertex_t;

typedef struct {
	uint32_t offset;
	uint32_t count;
} imdd_batch_t;

typedef struct {
	imdd_instance_transform_t *begin;
	imdd_instance_transform_t *current;
	imdd_instance_transform_t *end;
	imdd_instance_color_t *color;
} imdd_instance_stream_t;

typedef struct {
	imdd_array_filled_vertex_t *begin;
	imdd_array_filled_vertex_t *current;
	imdd_array_filled_vertex_t *end;
} imdd_filled_vertex_stream_t;

typedef struct {
	imdd_array_wire_vertex_t *begin;
	imdd_array_wire_vertex_t *current;
	imdd_array_wire_vertex_t *end;
} imdd_wire_vertex_stream_t;

static
void imdd_emit_line(imdd_wire_vertex_stream_t *stream, uint32_t col, imdd_v4 const *data)
{
	imdd_array_wire_vertex_t *const next = stream->current + 2;
	if (next > stream->end) {
		return;
	}

	imdd_v4 const start = data[0];
	imdd_v4 const end = data[1];

	imdd_v4 const tmp = imdd_v4_init_1f(imdd_asfloat(col));

	imdd_array_wire_vertex_t *const vertices = stream->current;
	vertices[0].pos_col = imdd_v4_set_w(start, tmp);
	vertices[1].pos_col = imdd_v4_set_w(end, tmp);
	stream->current = next;
}

static
void imdd_emit_wire_triangle(imdd_wire_vertex_stream_t *stream, uint32_t col, imdd_v4 const *data)
{
	imdd_array_wire_vertex_t *const next = stream->current + 6;
	if (next > stream->end) {
		return;
	}

	imdd_v4 const pos_a = data[0];
	imdd_v4 const pos_b = data[1];
	imdd_v4 const pos_c = data[2];

	imdd_v4 const tmp = imdd_v4_init_1f(imdd_asfloat(col));
	imdd_v4 const pos_col_a = imdd_v4_set_w(pos_a, tmp);
	imdd_v4 const pos_col_b = imdd_v4_set_w(pos_b, tmp);
	imdd_v4 const pos_col_c = imdd_v4_set_w(pos_c, tmp);

	imdd_array_wire_vertex_t *const vertices = stream->current;
	vertices[0].pos_col = pos_col_a;
	vertices[1].pos_col = pos_col_b;
	vertices[2].pos_col = pos_col_b;
	vertices[3].pos_col = pos_col_c;
	vertices[4].pos_col = pos_col_c;
	vertices[5].pos_col = pos_col_a;
	stream->current = next;
}

static
void imdd_emit_filled_triangle(imdd_filled_vertex_stream_t *stream, uint32_t col, imdd_v4 const *data)
{
	imdd_array_filled_vertex_t *const next = stream->current + 3;
	if (next > stream->end) {
		return;
	}

	imdd_v4 const pos_a = data[0];
	imdd_v4 const pos_b = data[1];
	imdd_v4 const pos_c = data[2];

	imdd_v4 const normal = imdd_v4_normalize3(imdd_v4_cross(
		imdd_v4_sub(pos_c, pos_a),
		imdd_v4_sub(pos_a, pos_b)));

	imdd_v4 const tmp = imdd_v4_init_1f(imdd_asfloat(col));

	imdd_array_filled_vertex_t *const vertices = stream->current;
	vertices[0].pos_col = imdd_v4_set_w(pos_a, tmp);
	vertices[0].normal_pad = normal;
	vertices[1].pos_col = imdd_v4_set_w(pos_b, tmp);
	vertices[1].normal_pad = normal;
	vertices[2].pos_col = imdd_v4_set_w(pos_c, tmp);
	vertices[2].normal_pad = normal;
	stream->current = next;
}

static
void imdd_emit_aabb(imdd_instance_stream_t *stream, uint32_t col, imdd_v4 const *data)
{
	if (stream->current == stream->end) {
		return;
	}

	imdd_v4 const min = data[0];
	imdd_v4 const max = data[1];

	imdd_v4 const half = imdd_v4_const_0_5f();
	imdd_v4 const centre = imdd_v4_mul(imdd_v4_add(max, min), half);
	imdd_v4 const half_extent = imdd_v4_mul(imdd_v4_sub(max, min), half);

	imdd_v4 const zero = imdd_v4_const_zero();
	imdd_v4 r0 = imdd_v4_set_x(zero, half_extent);
	imdd_v4 r1 = imdd_v4_set_y(zero, half_extent);
	imdd_v4 r2 = imdd_v4_set_z(zero, half_extent);
	imdd_v4 r3 = centre;

	imdd_v4_transpose_inplace(r0, r1, r2, r3);

	imdd_instance_transform_t *transform = stream->current++;
	imdd_instance_color_t *color = stream->color++;

	transform->row0 = r0;
	transform->row1 = r1;
	transform->row2 = r2;
	color->col = col;
}

static
void imdd_emit_transform(imdd_instance_stream_t *stream, uint32_t col, imdd_v4 const *data)
{
	if (stream->current == stream->end) {
		return;
	}

	imdd_instance_transform_t *transform = stream->current++;
	imdd_instance_color_t *color = stream->color++;

	transform->row0 = data[0];
	transform->row1 = data[1];
	transform->row2 = data[2];
	color->col = col;
}

static
void imdd_emit_sphere(imdd_instance_stream_t *stream, uint32_t col, imdd_v4 const *data)
{
	if (stream->current == stream->end) {
		return;
	}

	imdd_instance_transform_t *transform = stream->current++;
	imdd_instance_color_t *color = stream->color++;

	imdd_v4 const centre_radius = data[0];

	imdd_v4 const radius = imdd_v4_swiz_wwww(centre_radius);
	imdd_v4 const zero = imdd_v4_const_zero();
	imdd_v4 r0 = imdd_v4_set_x(zero, radius);
	imdd_v4 r1 = imdd_v4_set_y(zero, radius);
	imdd_v4 r2 = imdd_v4_set_z(zero, radius);
	imdd_v4 r3 = centre_radius;

	imdd_v4_transpose_inplace(r0, r1, r2, r3);

	transform->row0 = r0;
	transform->row1 = r1;
	transform->row2 = r2;
	color->col = col;
}

static inline
uint32_t imdd_instance_batch_index(imdd_mesh_enum_t mesh, imdd_style_enum_t style, imdd_blend_enum_t blend, imdd_zmode_enum_t zmode)
{
	return (mesh << 3) | (style << 2) | (blend << 1) | zmode;
}

static inline
uint32_t imdd_array_batch_index(imdd_blend_enum_t blend, imdd_zmode_enum_t zmode)
{
	return (blend << 1) | zmode;
}

#define IMDD_INSTANCE_BATCH_COUNT		(IMDD_MESH_COUNT << 3)
#define IMDD_ARRAY_BATCH_COUNT			4

typedef void (* imdd_emit_instance_func_t)(imdd_instance_stream_t *, uint32_t color, imdd_v4 const *);
typedef void (* imdd_emit_filled_vertex_func_t)(imdd_filled_vertex_stream_t *, uint32_t color, imdd_v4 const *);
typedef void (* imdd_emit_wire_vertex_func_t)(imdd_wire_vertex_stream_t *, uint32_t color, imdd_v4 const *);

typedef struct {
	imdd_emit_instance_func_t instance_func;
	imdd_emit_filled_vertex_func_t filled_vertex_func;
	imdd_emit_wire_vertex_func_t wire_vertex_func;
	uint32_t filled_vertex_count;
	uint32_t wire_vertex_count;
} imdd_emit_desc_t;

static imdd_emit_desc_t const g_imdd_emit_instance_desc[IMDD_SHAPE_COUNT] = {
	{ NULL, NULL, &imdd_emit_line, 0, 2 },									// IMDD_SHAPE_LINE
	{ NULL, &imdd_emit_filled_triangle, &imdd_emit_wire_triangle, 3, 6 },	// IMDD_SHAPE_TRIANGLE
	{ &imdd_emit_aabb, NULL, NULL, 0, 0 },									// IMDD_SHAPE_AABB
	{ &imdd_emit_transform, NULL, NULL, 0, 0 },								// IMDD_SHAPE_OBB
	{ &imdd_emit_sphere, NULL, NULL, 0, 0 },								// IMDD_SHAPE_SPHERE
	{ &imdd_emit_transform, NULL, NULL, 0, 0 },								// IMDD_SHAPE_ELLIPSOID
	{ &imdd_emit_transform, NULL, NULL, 0, 0 },								// IMDD_SHAPE_CONE
	{ &imdd_emit_transform, NULL, NULL, 0, 0 }								// IMDD_SHAPE_CYLINDER
};

static
void imdd_emit_shapes(
	imdd_shape_store_t const *const *stores,
	uint32_t store_count,

	imdd_instance_transform_t *instance_transform_buf,
	imdd_instance_color_t *instance_color_buf,
	uint32_t instance_capacity,
	imdd_batch_t *instance_batches,
	uint32_t *instance_count,

	imdd_array_filled_vertex_t *filled_vertex_buf,
	uint32_t filled_vertex_capacity,
	imdd_batch_t *filled_array_batches,
	uint32_t *filled_vertex_count,

	imdd_array_wire_vertex_t *wire_vertex_buf,
	uint32_t wire_vertex_capacity,
	imdd_batch_t *wire_array_batches,
	uint32_t *wire_vertex_count)
{
	uint32_t instance_counts[IMDD_INSTANCE_BATCH_COUNT];
	uint32_t filled_vertex_counts[IMDD_ARRAY_BATCH_COUNT];
	uint32_t wire_vertex_counts[IMDD_ARRAY_BATCH_COUNT];
	memset(instance_counts, 0, IMDD_INSTANCE_BATCH_COUNT*sizeof(uint32_t));
	memset(filled_vertex_counts, 0, IMDD_ARRAY_BATCH_COUNT*sizeof(uint32_t));
	memset(wire_vertex_counts, 0, IMDD_ARRAY_BATCH_COUNT*sizeof(uint32_t));

	// count vertices and instances
	for (uint32_t store_index = 0; store_index < store_count; ++store_index) {
		imdd_shape_store_t const *const store = stores[store_index];
		for (uint32_t bucket_index = 0; bucket_index < IMDD_SHAPE_BUCKET_COUNT; ++bucket_index) {
			imdd_shape_header_t const header = imdd_shape_header_from_bucket_index(bucket_index);
			if (header.shape >= IMDD_SHAPE_COUNT) {
				continue;
			}

			imdd_emit_desc_t const *const desc = g_imdd_emit_instance_desc + header.shape;
			imdd_style_enum_t const style = (imdd_style_enum_t)header.style;
			imdd_blend_enum_t const blend = (imdd_blend_enum_t)header.blend;
			imdd_zmode_enum_t const zmode = (imdd_zmode_enum_t)header.zmode;
			uint32_t const bucket_size = imdd_atomic_load(&store->bucket_sizes[bucket_index]);

			if (desc->instance_func) {
				imdd_mesh_enum_t const mesh = g_imdd_mesh_from_shape[header.shape];
				uint32_t const batch_index = imdd_instance_batch_index(mesh, style, blend, zmode);
				instance_counts[batch_index] += bucket_size;
			}
			if (desc->filled_vertex_func && style == IMDD_STYLE_FILLED) {
				uint32_t const batch_index = imdd_array_batch_index(blend, zmode);
				filled_vertex_counts[batch_index] += bucket_size*desc->filled_vertex_count;
			}
			if (desc->wire_vertex_func && style == IMDD_STYLE_WIRE) {
				uint32_t const batch_index = imdd_array_batch_index(blend, zmode);
				wire_vertex_counts[batch_index] += bucket_size*desc->wire_vertex_count;
			}
		}
	}

	// partition the buffers between draw calls
	imdd_instance_stream_t instance_streams[IMDD_INSTANCE_BATCH_COUNT];
	imdd_filled_vertex_stream_t filled_vertex_streams[IMDD_ARRAY_BATCH_COUNT];
	imdd_wire_vertex_stream_t wire_vertex_streams[IMDD_ARRAY_BATCH_COUNT];
	uint32_t end_offset;

	end_offset = 0;
	for (uint32_t batch_index = 0; batch_index < IMDD_INSTANCE_BATCH_COUNT; ++batch_index) {
		uint32_t const start_offset = end_offset;
		end_offset += instance_counts[batch_index];
		if (end_offset > instance_capacity) {
			end_offset = instance_capacity;
		}
		instance_streams[batch_index].begin = instance_transform_buf + start_offset;
		instance_streams[batch_index].current = instance_transform_buf + start_offset;
		instance_streams[batch_index].end = instance_transform_buf + end_offset;
		instance_streams[batch_index].color = instance_color_buf + start_offset;
	}
	*instance_count = end_offset;

	end_offset = 0;
	for (uint32_t batch_index = 0; batch_index < IMDD_ARRAY_BATCH_COUNT; ++batch_index) {
		uint32_t const start_offset = end_offset;
		end_offset += filled_vertex_counts[batch_index];
		if (end_offset > filled_vertex_capacity) {
			end_offset = filled_vertex_capacity;
		}
		filled_vertex_streams[batch_index].begin = filled_vertex_buf + start_offset;
		filled_vertex_streams[batch_index].current = filled_vertex_buf + start_offset;
		filled_vertex_streams[batch_index].end = filled_vertex_buf + end_offset;
	}
	*filled_vertex_count = end_offset;

	end_offset = 0;
	for (uint32_t batch_index = 0; batch_index < IMDD_ARRAY_BATCH_COUNT; ++batch_index) {
		uint32_t const start_offset = end_offset;
		end_offset += wire_vertex_counts[batch_index];
		if (end_offset > wire_vertex_capacity) {
			end_offset = wire_vertex_capacity;
		}
		wire_vertex_streams[batch_index].begin = wire_vertex_buf + start_offset;
		wire_vertex_streams[batch_index].current = wire_vertex_buf + start_offset;
		wire_vertex_streams[batch_index].end = wire_vertex_buf + end_offset;
	}
	*wire_vertex_count = end_offset;

	// write the vertices through the streams
	for (uint32_t store_index = 0; store_index < store_count; ++store_index) {
		imdd_shape_store_t const *const store = stores[store_index];
		uint32_t header_count = imdd_atomic_load(&store->header_count);
		for (uint32_t header_offset = 0; header_offset < header_count; ++header_offset) {
			imdd_shape_header_t const header = store->header_store[header_offset];
			if (header.shape >= IMDD_SHAPE_COUNT) {
				continue;
			}

			imdd_emit_desc_t const *const desc = g_imdd_emit_instance_desc + header.shape;
			imdd_style_enum_t const style = (imdd_style_enum_t)header.style;
			imdd_blend_enum_t const blend = (imdd_blend_enum_t)header.blend;
			imdd_zmode_enum_t const zmode = (imdd_zmode_enum_t)header.zmode;
			imdd_v4 const *data = store->data_qw_store + header.data_qw_offset;

			if (desc->instance_func) {
				imdd_mesh_enum_t const mesh = g_imdd_mesh_from_shape[header.shape];
				uint32_t const batch_index = imdd_instance_batch_index(mesh, style, blend, zmode);
				desc->instance_func(instance_streams + batch_index, header.color, data);
			}
			if (desc->filled_vertex_func && style == IMDD_STYLE_FILLED) {
				uint32_t const batch_index = imdd_array_batch_index(blend, zmode);
				desc->filled_vertex_func(filled_vertex_streams + batch_index, header.color, data);
			}
			if (desc->wire_vertex_func && style == IMDD_STYLE_WIRE) {
				uint32_t const batch_index = imdd_array_batch_index(blend, zmode);
				desc->wire_vertex_func(wire_vertex_streams + batch_index, header.color, data);
			}
		}
	}

	// write out the draw calls
	for (uint32_t batch_index = 0; batch_index < IMDD_INSTANCE_BATCH_COUNT; ++batch_index) {
		imdd_instance_stream_t const *const stream = &instance_streams[batch_index];
		instance_batches[batch_index].offset = (uint32_t)(stream->begin - instance_transform_buf);
		instance_batches[batch_index].count = (uint32_t)(stream->current - stream->begin);
	}
	for (uint32_t batch_index = 0; batch_index < IMDD_ARRAY_BATCH_COUNT; ++batch_index) {
		imdd_filled_vertex_stream_t const *const stream = &filled_vertex_streams[batch_index];
		filled_array_batches[batch_index].offset = (uint32_t)(stream->begin - filled_vertex_buf);
		filled_array_batches[batch_index].count = (uint32_t)(stream->current - stream->begin);
	}
	for (uint32_t batch_index = 0; batch_index < IMDD_ARRAY_BATCH_COUNT; ++batch_index) {
		imdd_wire_vertex_stream_t const *const stream = &wire_vertex_streams[batch_index];
		wire_array_batches[batch_index].offset = (uint32_t)(stream->begin - wire_vertex_buf);
		wire_array_batches[batch_index].count = (uint32_t)(stream->current - stream->begin);
	}
}

#ifdef __cplusplus
} // extern "C"
#endif
