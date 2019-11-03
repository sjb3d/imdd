#pragma once

#include <math.h>

#define IMDD_VECTORCALL

typedef struct imdd_v4 {
	float x, y, z, w;
} imdd_v4;

static inline
imdd_v4 imdd_v4_init_4f(float x, float y, float z, float w)
{
	imdd_v4 v;
	v.x = x;
	v.y = y;
	v.z = z;
	v.w = w;
	return v;
}
static inline
imdd_v4 imdd_v4_init_3f(float x, float y, float z)
{
	imdd_v4 v;
	v.x = x;
	v.y = y;
	v.z = z;
	v.w = 0.f;
	return v;
}
static inline
imdd_v4 imdd_v4_init_1f(float s)
{
	imdd_v4 v;
	v.x = s;
	v.y = s;
	v.z = s;
	v.w = s;
	return v;
}

#define imdd_v4_const_zero()		imdd_v4_init_1f(0.f)
#define imdd_v4_const_0_5f()		imdd_v4_init_1f(.5f)

static inline
imdd_v4 imdd_v4_load_3f(float const *p)
{
	return imdd_v4_init_3f(p[0], p[1], p[2]);
}

static inline
void imdd_v4_store_3f(float *p, imdd_v4 v)
{
	p[0] = v.x;
	p[1] = v.y;
	p[2] = v.z;
}

#define imdd_v4_swiz_xxxx(v)		imdd_v4_init_1f((v).x)
#define imdd_v4_swiz_yyyy(v)		imdd_v4_init_1f((v).y)
#define imdd_v4_swiz_zzzz(v)		imdd_v4_init_1f((v).z)
#define imdd_v4_swiz_wwww(v)		imdd_v4_init_1f((v).w)

#define imdd_v4_transpose_inplace(io0, io1, io2, io3)		\
	do {													\
		imdd_v4 const t0 = io0;								\
		imdd_v4 const t1 = io1;								\
		imdd_v4 const t2 = io2;								\
		imdd_v4 const t3 = io3;								\
		io0 = imdd_v4_init_4f(t0.x, t1.x, t2.x, t3.x);		\
		io1 = imdd_v4_init_4f(t0.y, t1.y, t2.y, t3.y);		\
		io2 = imdd_v4_init_4f(t0.z, t1.z, t2.z, t3.z);		\
	} while(0);

static inline imdd_v4 imdd_v4_set_x(imdd_v4 a, imdd_v4 b) { a.x = b.x; return a; }
static inline imdd_v4 imdd_v4_set_y(imdd_v4 a, imdd_v4 b) { a.y = b.y; return a; }
static inline imdd_v4 imdd_v4_set_z(imdd_v4 a, imdd_v4 b) { a.z = b.z; return a; }
static inline imdd_v4 imdd_v4_set_w(imdd_v4 a, imdd_v4 b) { a.w = b.w; return a; }

static inline
imdd_v4 imdd_v4_mul_sign(imdd_v4 a, imdd_v4 b)
{
	imdd_v4 c;
	c.x = imdd_asfloat(imdd_asuint(a.x) ^ (imdd_asuint(b.x) & 0x80000000U));
	c.y = imdd_asfloat(imdd_asuint(a.y) ^ (imdd_asuint(b.y) & 0x80000000U));
	c.z = imdd_asfloat(imdd_asuint(a.z) ^ (imdd_asuint(b.z) & 0x80000000U));
	c.w = imdd_asfloat(imdd_asuint(a.w) ^ (imdd_asuint(b.w) & 0x80000000U));
	return c;
}

#define IMDD_V4_OP_IMPL(OP)		\
	imdd_v4 c;					\
	c.x = a.x OP b.x;			\
	c.y = a.y OP b.y;			\
	c.z = a.z OP b.z;			\
	c.w = a.w OP b.w;			\
	return c;

static inline imdd_v4 imdd_v4_add(imdd_v4 a, imdd_v4 b)	{ IMDD_V4_OP_IMPL(+) }
static inline imdd_v4 imdd_v4_sub(imdd_v4 a, imdd_v4 b)	{ IMDD_V4_OP_IMPL(-) }
static inline imdd_v4 imdd_v4_mul(imdd_v4 a, imdd_v4 b)	{ IMDD_V4_OP_IMPL(*) }
static inline imdd_v4 imdd_v4_div(imdd_v4 a, imdd_v4 b)	{ IMDD_V4_OP_IMPL(/) }

static inline
imdd_v4 imdd_v4_dot3(imdd_v4 a, imdd_v4 b)
{
	return imdd_v4_init_1f(a.x*b.x + a.y*b.y + a.z*b.z);
}

static inline
imdd_v4 imdd_v4_cross(imdd_v4 a, imdd_v4 b)
{
	return imdd_v4_init_3f(
		a.y*b.z - a.z*b.y,
		a.z*b.x - a.x*b.z,
		a.x*b.y - a.y*b.x);
}

static inline
imdd_v4 imdd_v4_normalize3(imdd_v4 a)
{
	float const len = sqrtf(a.x*a.x + a.y*a.y + a.z*a.z);
	return imdd_v4_init_3f(a.x/len, a.y/len, a.z/len);
}
