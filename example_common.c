#include "example_common.h"
#include <math.h>
#include <string.h>

#define vec4(x, y, z, w)	imdd_v4_init_4f((x), (y), (z), (w))
#define vec3(x, y, z)		imdd_v4_init_3f((x), (y), (z))

static inline
void mat4_set_col(mat4 *d, uint32_t i, float x, float y, float z, float w)
{
	d->m[i][0] = x;
	d->m[i][1] = y;
	d->m[i][2] = z;
	d->m[i][3] = w;
}

static inline
void mat4_mul(mat4 *d, mat4 const *a, mat4 const *b)
{
	for (uint32_t y = 0; y < 4; ++y)
	for (uint32_t x = 0; x < 4; ++x) {
		float t = 0.f;
		for (uint32_t i = 0; i < 4; ++i) {
			t += a->m[x][i]*b->m[i][y];
		}
		d->m[x][y] = t;
	}
}

void mat4_identity(mat4 *d)
{
	mat4_set_col(d, 0, 1.f, 0.f, 0.f, 0.f);
	mat4_set_col(d, 1, 0.f, 1.f, 0.f, 0.f);
	mat4_set_col(d, 2, 0.f, 0.f, 1.f, 0.f);
	mat4_set_col(d, 3, 0.f, 0.f, 0.f, 1.f);
}

void mat4_rotate_x(mat4 *d, float angle)
{
	mat4 a;
	memcpy(&a, d, sizeof(mat4));

	float const c = cosf(angle);
	float const s = sinf(angle);

	mat4 b;
	mat4_set_col(&b, 0, 1.f, 0.f, 0.f, 0.f);
	mat4_set_col(&b, 1, 0.f,   c,   s, 0.f);
	mat4_set_col(&b, 2, 0.f,  -s,   c, 0.f);
	mat4_set_col(&b, 3, 0.f, 0.f, 0.f, 1.f);

	mat4_mul(d, &a, &b);
}

void mat4_rotate_y(mat4 *d, float angle)
{
	mat4 a;
	memcpy(&a, d, sizeof(mat4));

	float const c = cosf(angle);
	float const s = sinf(angle);

	mat4 b;
	mat4_set_col(&b, 0,   c, 0.f,   s, 0.f);
	mat4_set_col(&b, 1, 0.f, 1.f, 0.f, 0.f);
	mat4_set_col(&b, 2,  -s, 0.f,   c, 0.f);
	mat4_set_col(&b, 3, 0.f, 0.f, 0.f, 1.f);

	mat4_mul(d, &a, &b);
}

void mat4_translation(mat4 *d, float x, float y, float z)
{
	mat4 a;
	memcpy(&a, d, sizeof(mat4));

	mat4 b;
	mat4_set_col(&b, 0, 1.f, 0.f, 0.f, 0.f);
	mat4_set_col(&b, 1, 0.f, 1.f, 0.f, 0.f);
	mat4_set_col(&b, 2, 0.f, 0.f, 1.f, 0.f);
	mat4_set_col(&b, 3,   x,   y,   z, 1.f);

	mat4_mul(d, &a, &b);
}

void mat4_perspective_gl(mat4 *d, float fov_y, float aspect, float z_near, float z_far)
{
	mat4 a;
	memcpy(&a, d, sizeof(mat4));

	float const cot = 1.f/tanf(.5f*fov_y);
	float const r = 1.f/(z_near - z_far);

	mat4 b;
	mat4_set_col(&b, 0, cot/aspect, 0.f, 0.f, 0.f);
	mat4_set_col(&b, 1, 0.f, cot, 0.f, 0.f);
	mat4_set_col(&b, 2, 0.f, 0.f, (z_far + z_near)*r, -1.f);
	mat4_set_col(&b, 3, 0.f, 0.f, 2.f*z_far*z_near*r, 0.f);

	mat4_mul(d, &a, &b);
}

void mat4_perspective_vk(mat4 *d, float fov_y, float aspect, float z_near, float z_far)
{
	mat4 a;
	memcpy(&a, d, sizeof(mat4));

	float const cot = 1.f/tanf(.5f*fov_y);
	float const r = 1.f/(z_near - z_far);

	mat4 b;
	mat4_set_col(&b, 0, cot/aspect, 0.f, 0.f, 0.f);
	mat4_set_col(&b, 1, 0.f, -cot, 0.f, 0.f);
	mat4_set_col(&b, 2, 0.f, 0.f, (z_far + z_near)*r, -1.f);
	mat4_set_col(&b, 3, 0.f, 0.f, 2.f*z_far*z_near*r, 0.f);

	mat4_mul(d, &a, &b);
}

uint32_t test_alpha_from_blend(int blend)
{
	return (blend == IMDD_BLEND_OPAQUE ? 0xffU : 0x7f) << 24;
}

void test_line(imdd_shape_store_t *store, imdd_v4 centre, int style, int blend)
{
	if (style != IMDD_STYLE_WIRE) {
		return;
	}
	uint32_t const alpha = test_alpha_from_blend(blend);
	imdd_line(
		store,
		IMDD_ZMODE_TEST,
		imdd_v4_add(centre, vec3(-1.f, 0.f, 0.f)),
		imdd_v4_add(centre, vec3( 1.f, 0.f, 0.f)),
		alpha | 0x0000ffU);
	imdd_line(
		store,
		IMDD_ZMODE_TEST,
		imdd_v4_add(centre, vec3(0.f, -1.f, 0.f)),
		imdd_v4_add(centre, vec3(0.f,  1.f, 0.f)),
		alpha | 0x00ff00U);
	imdd_line(
		store,
		IMDD_ZMODE_TEST,
		imdd_v4_add(centre, vec3(0.f, 0.f, -1.f)),
		imdd_v4_add(centre, vec3(0.f, 0.f,  1.f)),
		alpha | 0xff0000U);
}

void test_triangle(imdd_shape_store_t *store, imdd_v4 centre, int style, int blend)
{
	imdd_v4 const quad_a = imdd_v4_add(centre, vec3(-1.f,  .5f, -1.f));
	imdd_v4 const quad_b = imdd_v4_add(centre, vec3( 1.f, -.5f, -1.f));
	imdd_v4 const quad_c = imdd_v4_add(centre, vec3(-1.f, -.5f,  1.f));
	imdd_v4 const quad_d = imdd_v4_add(centre, vec3( 1.f,  .5f,  1.f));

	uint32_t const col = test_alpha_from_blend(blend) | 0x7f7fffU;

	imdd_triangle(store, style, IMDD_ZMODE_TEST, quad_a, quad_d, quad_b, col);
	imdd_triangle(store, style, IMDD_ZMODE_TEST, quad_c, quad_d, quad_a, col);
}

void test_aabb(imdd_shape_store_t *store, imdd_v4 centre, int style, int blend)
{
	imdd_v4 const half_extents = imdd_v4_init_1f(1.f);
	imdd_aabb(
		store,
		style,
		IMDD_ZMODE_TEST,
		imdd_v4_sub(centre, half_extents),
		imdd_v4_add(centre, half_extents),
		test_alpha_from_blend(blend) | 0xff7fffU);
}

void test_obb(imdd_shape_store_t *store, imdd_v4 centre, int style, int blend)
{
	float const r2 = sqrtf(1.f/2.f);
	float const r3 = sqrtf(1.f/3.f);
	imdd_v4 const a0 = vec3(r3, r3, r3);
	imdd_v4 const a1 = vec3(r2, -r2, 0.f);
	imdd_v4 const a2 = imdd_v4_normalize3(imdd_v4_cross(a0, a1));
	imdd_obb(
		store,
		style,
		IMDD_ZMODE_TEST,
		imdd_v4_mul(imdd_v4_init_1f(.8f), a0),
		imdd_v4_mul(imdd_v4_init_1f(.5f), a1),
		imdd_v4_mul(imdd_v4_init_1f(.7f), a2),
		centre,
		test_alpha_from_blend(blend) | 0xffff7fU);
}

void test_sphere(imdd_shape_store_t *store, imdd_v4 centre, int style, int blend)
{
	imdd_sphere(
		store,
		style,
		IMDD_ZMODE_TEST,
		imdd_v4_set_w(centre, imdd_v4_init_1f(1.f)),
		test_alpha_from_blend(blend) | 0x7fffffU);
}

void test_ellipsoid(imdd_shape_store_t *store, imdd_v4 centre, int style, int blend)
{
	float const r2 = sqrtf(1.f/2.f);
	float const r3 = sqrtf(1.f/3.f);
	imdd_v4 const a0 = vec3(r3, r3, r3);
	imdd_v4 const a1 = vec3(r2, -r2, 0.f);
	imdd_v4 const a2 = imdd_v4_normalize3(imdd_v4_cross(a0, a1));
	imdd_ellipsoid(
		store,
		style,
		IMDD_ZMODE_TEST,
		imdd_v4_mul(imdd_v4_init_1f(.8f), a0),
		imdd_v4_mul(imdd_v4_init_1f(.5f), a1),
		imdd_v4_mul(imdd_v4_init_1f(.7f), a2),
		centre,
		test_alpha_from_blend(blend) | 0xff7f7fU);
}

void test_cone(imdd_shape_store_t *store, imdd_v4 centre, int style, int blend)
{
	float const r2 = sqrtf(1.f/2.f);
	float const r3 = sqrtf(1.f/3.f);
	imdd_v4 const a0 = vec3(r3, r3, r3);
	imdd_v4 const a1 = vec3(r2, -r2, 0.f);
	imdd_v4 const a2 = imdd_v4_normalize3(imdd_v4_cross(a0, a1));
	imdd_v4 const z_axis = imdd_v4_mul(imdd_v4_init_1f(.7f), a2);
	imdd_cone(
		store,
		style,
		IMDD_ZMODE_TEST,
		imdd_v4_mul(imdd_v4_init_1f(.8f), a0),
		imdd_v4_mul(imdd_v4_init_1f(.5f), a1),
		imdd_v4_mul(z_axis, imdd_v4_init_1f(2.f)),
		imdd_v4_sub(centre, z_axis),
		test_alpha_from_blend(blend) | 0x7fff7fU);
}

void test_frustum(imdd_shape_store_t *store, imdd_v4 centre, int style, int blend)
{
	float const r2 = sqrtf(1.f/2.f);
	float const r3 = sqrtf(1.f/3.f);
	imdd_v4 const a0 = vec3(r3, r3, r3);
	imdd_v4 const a1 = vec3(r2, -r2, 0.f);
	imdd_v4 const a2 = imdd_v4_normalize3(imdd_v4_cross(a0, a1));
	imdd_v4 const z_axis = imdd_v4_mul(imdd_v4_init_1f(.7f), a2);
	imdd_frustum(
		store,
		style,
		IMDD_ZMODE_TEST,
		imdd_v4_mul(imdd_v4_init_1f(.8f), a0),
		imdd_v4_mul(imdd_v4_init_1f(.5f), a1),
		imdd_v4_mul(z_axis, imdd_v4_init_1f(2.f)),
		imdd_v4_sub(centre, z_axis),
		test_alpha_from_blend(blend) | 0xff3f7fU);
}

void test_cylinder(imdd_shape_store_t *store, imdd_v4 centre, int style, int blend)
{
	float const r2 = sqrtf(1.f/2.f);
	float const r3 = sqrtf(1.f/3.f);
	imdd_v4 const a0 = vec3(r3, r3, r3);
	imdd_v4 const a1 = vec3(r2, -r2, 0.f);
	imdd_v4 const a2 = imdd_v4_normalize3(imdd_v4_cross(a0, a1));
	imdd_cylinder(
		store,
		style,
		IMDD_ZMODE_TEST,
		imdd_v4_mul(imdd_v4_init_1f(.8f), a0),
		imdd_v4_mul(imdd_v4_init_1f(.5f), a1),
		imdd_v4_mul(imdd_v4_init_1f(.7f), a2),
		centre,
		test_alpha_from_blend(blend) | 0xff7f3fU);
}

typedef void (* test_func_t)(imdd_shape_store_t *, imdd_v4, int, int);

static test_func_t const g_test_func[] = {
	&test_line,
	&test_triangle,
	&test_aabb,
	&test_obb,
	&test_sphere,
	&test_ellipsoid,
	&test_cone,
	&test_frustum,
	&test_cylinder
};

void imdd_example_test(imdd_shape_store_t *store)
{
	int const test_count = IMDD_STYLE_COUNT*sizeof(g_test_func)/sizeof(g_test_func[0]);
	int const nx = (int)sqrtf((float)test_count);
	int const nz = (test_count + nx - 1)/nx;

	for (int iz = 0; iz < nz; ++iz)
	for (int ix = 0; ix < nx; ++ix) {
		int test_index = iz*nx + ix;
		if (test_index >= test_count) {
			break;
		}
		float const spacing = 2.5f;
		float const x = spacing*.5f*(float)(2*ix + 1 - nx);
		float const z = spacing*.5f*(float)(2*iz + 1 - nz);
		for (int blend = 0; blend < IMDD_BLEND_COUNT; ++blend) {
			float const y = spacing*.5f*(2*blend + 1 - IMDD_BLEND_COUNT);
			g_test_func[test_index/IMDD_STYLE_COUNT](store, vec3(x, y, z), test_index % IMDD_STYLE_COUNT, blend);
		}
	}
}

void imdd_perf_test(imdd_shape_store_t *store)
{
	float const sz = 40.f;
	imdd_v4 const scale = imdd_v4_init_1f(.15f);
	imdd_v4 const offset = imdd_v4_init_1f(-3.f);
	imdd_v4 const half_size = imdd_v4_init_1f(.05f);
	uint32_t i = 0;
	for (float z = 0.f; z < sz; z += 1.f)
	for (float y = 0.f; y < sz; y += 1.f)
	for (float x = 0.f; x < sz; x += 1.f) {
		imdd_v4 const centre = imdd_v4_add(imdd_v4_mul(imdd_v4_init_3f(x, y, z), scale), offset);
		imdd_style_enum_t const style = (i & 1) ? IMDD_STYLE_FILLED : IMDD_STYLE_WIRE;
		uint32_t const col = (i & 2) ? 0xffff7f00U : 0xff007fffU;
		imdd_aabb(
			store,
			style,
			IMDD_ZMODE_TEST,
		 	imdd_v4_sub(centre, half_size),
		 	imdd_v4_add(centre, half_size),
		 	col);
		++i;
	}
}
