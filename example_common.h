#pragma once

#include "imdd.h"

#define PI			3.1415926535f

#define UNUSED(X)	(void)X

typedef struct {
	float m[4][4];	// column major so [col][row]
} mat4;

void mat4_identity(mat4 *d);
void mat4_rotate_x(mat4 *d, float angle);
void mat4_rotate_y(mat4 *d, float angle);
void mat4_translation(mat4 *d, float x, float y, float z);
void mat4_perspective_gl(mat4 *d, float fov_y, float aspect, float z_near, float z_far);
void mat4_perspective_vk(mat4 *d, float fov_y, float aspect, float z_near, float z_far);

void imdd_example_test(imdd_shape_store_t *store);
void imdd_perf_test(imdd_shape_store_t *store);
