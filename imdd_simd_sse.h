#pragma once

#include <xmmintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>

#ifdef _MSC_VER
#define IDMD_SSE_ALIGN(DECL)	__declspec(align(16)) DECL
#define IMDD_VECTORCALL			__vectorcall
#else
#define IDMD_SSE_ALIGN(DECL)	DECL __attribute__((aligned(16)))
#define IMDD_VECTORCALL
#endif

typedef __m128 imdd_v4;

#define imdd_v4_init_4f(x, y, z, w)		_mm_set_ps((w), (z), (y), (x))
#define imdd_v4_init_3f(x, y, z)		_mm_set_ps(0.f, (z), (y), (x))
#define imdd_v4_init_1f(s)				_mm_set1_ps((s))

static IDMD_SSE_ALIGN(float const imdd_v4_data_zero[4]) = { 0.f, 0.f, 0.f, 0.f };
static IDMD_SSE_ALIGN(float const imdd_v4_data_0_5f[4]) = { .5f, .5f, .5f, .5f };
static IDMD_SSE_ALIGN(uint32_t const imdd_v4_data_signbit[4]) = { 0x80000000U, 0x80000000U, 0x80000000U, 0x80000000U };
static IDMD_SSE_ALIGN(uint32_t const imdd_v4_data_mask_x[4]) = { 0xffffffffU, 0, 0, 0 };
static IDMD_SSE_ALIGN(uint32_t const imdd_v4_data_mask_y[4]) = { 0, 0xffffffffU, 0, 0 };
static IDMD_SSE_ALIGN(uint32_t const imdd_v4_data_mask_z[4]) = { 0, 0, 0xffffffffU, 0 };
static IDMD_SSE_ALIGN(uint32_t const imdd_v4_data_mask_w[4]) = { 0, 0, 0, 0xffffffffU };

#define imdd_v4_const_zero()		(*(imdd_v4 *)imdd_v4_data_zero)
#define imdd_v4_const_0_5f()		(*(imdd_v4 *)imdd_v4_data_0_5f)
#define imdd_v4_const_signbit()		(*(imdd_v4 *)imdd_v4_data_signbit)
#define imdd_v4_const_mask_x()		(*(imdd_v4 *)imdd_v4_data_mask_x)
#define imdd_v4_const_mask_y()		(*(imdd_v4 *)imdd_v4_data_mask_y)
#define imdd_v4_const_mask_z()		(*(imdd_v4 *)imdd_v4_data_mask_z)
#define imdd_v4_const_mask_w()		(*(imdd_v4 *)imdd_v4_data_mask_w)

static inline
imdd_v4 imdd_v4_load_3f(float const *p)		{ return _mm_set_ps(0.f, p[2], p[1], p[0]); }

static inline
void imdd_v4_store_3f(float *p, imdd_v4 v)
{
	IDMD_SSE_ALIGN(float s[4]);
	_mm_store_ps(s, v);
	p[0] = s[0];
	p[1] = s[1];
	p[2] = s[2];
}

#define imdd_v4_shuf_code(x, y, z, w)	((x) | ((y) << 2) | ((z) << 4) | ((w) << 6))

static inline imdd_v4 imdd_v4_swiz_xxxx(imdd_v4 v)	{ return _mm_shuffle_ps(v, v, imdd_v4_shuf_code(0, 0, 0, 0)); }
static inline imdd_v4 imdd_v4_swiz_yyyy(imdd_v4 v)	{ return _mm_shuffle_ps(v, v, imdd_v4_shuf_code(1, 1, 1, 1)); }
static inline imdd_v4 imdd_v4_swiz_zzzz(imdd_v4 v)	{ return _mm_shuffle_ps(v, v, imdd_v4_shuf_code(2, 2, 2, 2)); }
static inline imdd_v4 imdd_v4_swiz_wwww(imdd_v4 v)	{ return _mm_shuffle_ps(v, v, imdd_v4_shuf_code(3, 3, 3, 3)); }
static inline imdd_v4 imdd_v4_swiz_yzxw(imdd_v4 v)	{ return _mm_shuffle_ps(v, v, imdd_v4_shuf_code(1, 2, 0, 3)); }

#define imdd_v4_swiz_xyab(xyzw, abcd)	_mm_movelh_ps((xyzw), (abcd))
#define imdd_v4_swiz_zwcd(xyzw, abcd)	_mm_movehl_ps((abcd), (xyzw))
#define imdd_v4_swiz_xayb(xyzw, abcd)	_mm_unpacklo_ps((xyzw), (abcd))
#define imdd_v4_swiz_zcwd(xyzw, abcd)	_mm_unpackhi_ps((xyzw), (abcd))

#define imdd_v4_transpose_stage1(t0, t1, t2, t3, i0, i1, i2, i3)	\
	do {															\
		(t0) = imdd_v4_swiz_xayb((i0), (i1));						\
		(t1) = imdd_v4_swiz_xayb((i2), (i3));						\
		(t2) = imdd_v4_swiz_zcwd((i0), (i1));						\
		(t3) = imdd_v4_swiz_zcwd((i2), (i3));						\
	} while(0);
#define imdd_v4_transpose_stage2(o0, o1, o2, o3, t0, t1, t2, t3)	\
	do {															\
		(o0) = imdd_v4_swiz_xyab((t0), (t1));						\
		(o1) = imdd_v4_swiz_zwcd((t0), (t1));						\
		(o2) = imdd_v4_swiz_xyab((t2), (t3));						\
		(o3) = imdd_v4_swiz_zwcd((t2), (t3));						\
	} while(0);
#define imdd_v4_transpose_inplace(io0, io1, io2, io3)							\
	do {																		\
		imdd_v4 t0, t1, t2, t3;													\
		imdd_v4_transpose_stage1(t0, t1, t2, t3, (io0), (io1), (io2), (io3));	\
		imdd_v4_transpose_stage2((io0), (io1), (io2), (io3), t0, t1, t2, t3);	\
	} while(0);

static inline
imdd_v4 imdd_v4_select(imdd_v4 mask, imdd_v4 true_val, imdd_v4 false_val)
{
	return _mm_or_ps(_mm_and_ps(mask, true_val), _mm_andnot_ps(mask, false_val));
}

#define imdd_v4_set_x(v, s)		imdd_v4_select(imdd_v4_const_mask_x(), (s), (v))
#define imdd_v4_set_y(v, s)		imdd_v4_select(imdd_v4_const_mask_y(), (s), (v))
#define imdd_v4_set_z(v, s)		imdd_v4_select(imdd_v4_const_mask_z(), (s), (v))
#define imdd_v4_set_w(v, s)		imdd_v4_select(imdd_v4_const_mask_w(), (s), (v))

#define imdd_v4_mul_sign(a, b)	_mm_xor_ps((a), _mm_and_ps((b), imdd_v4_const_signbit()))

#define imdd_v4_add(a, b)		_mm_add_ps((a), (b))
#define imdd_v4_sub(a, b)		_mm_sub_ps((a), (b))
#define imdd_v4_mul(a, b)		_mm_mul_ps((a), (b))
#define imdd_v4_div(a, b)		_mm_div_ps((a), (b))

static inline
imdd_v4 imdd_v4_dot3(imdd_v4 a, imdd_v4 b)
{
	imdd_v4 const ab = _mm_mul_ps(a, b);
	imdd_v4 p = imdd_v4_swiz_xxxx(ab);
	p = _mm_add_ps(p, imdd_v4_swiz_yyyy(ab));
	p = _mm_add_ps(p, imdd_v4_swiz_zzzz(ab));
	return p;
}

static inline
imdd_v4 imdd_v4_cross(imdd_v4 a, imdd_v4 b)
{
	imdd_v4 const t0 = _mm_mul_ps(a, imdd_v4_swiz_yzxw(b));
	imdd_v4 const t1 = _mm_mul_ps(imdd_v4_swiz_yzxw(a), b);
	return imdd_v4_swiz_yzxw(_mm_sub_ps(t0, t1));
}

static inline
imdd_v4 imdd_v4_normalize3(imdd_v4 a)
{
	imdd_v4 const len_sq = imdd_v4_dot3(a, a);
	imdd_v4 const len = imdd_v4_swiz_xxxx(_mm_sqrt_ss(len_sq));
	return _mm_div_ps(a, len);
}
