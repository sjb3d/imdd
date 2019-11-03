#pragma once

#include "imdd.h"

#ifdef __cplusplus
extern "C" {
#endif

// low 8 bits of the structure is the bucket index (used for batching)
typedef struct {
	uint32_t style : 1;
	uint32_t zmode : 1;
	uint32_t blend : 1;
	uint32_t shape : 5;
	uint32_t data_qw_offset : 24;
	uint32_t color;
} imdd_shape_header_t;

static inline
uint32_t imdd_bucket_index_from_shape_header(imdd_shape_header_t header)
{
	union { imdd_shape_header_t header; uint32_t bits[2]; } u;
	u.header = header;
	return u.bits[0] & 0xffU;
}

static inline
imdd_shape_header_t imdd_shape_header_from_bucket_index(uint32_t index)
{
	union { imdd_shape_header_t header; uint32_t bits[2]; } u;
	u.bits[0] = index;
	u.bits[1] = 0;
	return u.header;
}

#define IMDD_SHAPE_BUCKET_COUNT		(IMDD_SHAPE_COUNT << 3)

struct imdd_shape_store_tag {
	imdd_shape_header_t *header_store;
	imdd_atomic_uint header_count;
	uint32_t header_capacity;
	imdd_atomic_uint bucket_sizes[IMDD_SHAPE_BUCKET_COUNT];

	imdd_v4 *data_qw_store;
	imdd_atomic_uint data_qw_count;
	uint32_t data_qw_capacity;
};

#ifdef __cplusplus
} // extern "C"
#endif
