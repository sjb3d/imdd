#pragma once

static inline
float imdd_asfloat(uint32_t i)
{
	union { float f; uint32_t i; } u;
	u.i = i;
	return u.f;
}

static inline
uint32_t imdd_asuint(float f)
{
	union { float f; uint32_t i; } u;
	u.f = f;
	return u.i;
}

#if defined(IMDD_NO_SIMD)
#include "imdd_simd_fallback.h"
#else
#include "imdd_simd_sse.h"
#endif
