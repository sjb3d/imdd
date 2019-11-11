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

#define IMDD_CACHE_LINE_SIZE	64

struct imdd_shape_store_tag {
	imdd_atomic_uint header_count;
	uint8_t header_padding[IMDD_CACHE_LINE_SIZE - sizeof(imdd_atomic_uint)];

	imdd_atomic_uint data_qw_count;
	uint8_t data_qw_padding[IMDD_CACHE_LINE_SIZE - sizeof(imdd_atomic_uint)];

	imdd_shape_header_t *header_store;
	imdd_v4 *data_qw_store;
	uint32_t header_capacity;
	uint32_t data_qw_capacity;
};

#ifdef __cplusplus
} // extern "C"
#endif
