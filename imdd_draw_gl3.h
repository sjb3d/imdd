#pragma once

#include "imdd_draw_util.h"
#include <stdlib.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	GLuint prog;
	GLint proj_from_world_loc;
} imdd_gl3_program_t;

typedef struct {
	imdd_mesh_layout_t layout;
	GLuint vertex_buf;
	GLuint index_buf;
	GLenum draw_mode;
} imdd_gl3_mesh_buffer_t;

typedef struct {
	imdd_gl3_program_t instance_program[IMDD_STYLE_COUNT];
	imdd_gl3_program_t array_program[IMDD_STYLE_COUNT];

	imdd_gl3_mesh_buffer_t mesh_buffer[IMDD_STYLE_COUNT];
	GLuint instance_transform_buf;
	GLuint instance_color_buf;
	GLuint instance_vertex_array[IMDD_STYLE_COUNT];
	GLuint filled_vertex_buf;
	GLuint filled_vertex_array;
	GLuint wire_vertex_buf;
	GLuint wire_vertex_array;

	imdd_instance_transform_t *instance_transform_staging;
	imdd_instance_color_t *instance_color_staging;
	uint32_t instance_capacity;
	imdd_array_filled_vertex_t *filled_vertex_staging;
	uint32_t filled_vertex_capacity;
	imdd_array_wire_vertex_t *wire_vertex_staging;
	uint32_t wire_vertex_capacity;

	imdd_batch_t instance_batches[IMDD_INSTANCE_BATCH_COUNT];
	imdd_batch_t filled_array_batches[IMDD_ARRAY_BATCH_COUNT];
	imdd_batch_t wire_array_batches[IMDD_ARRAY_BATCH_COUNT];
} imdd_gl3_context_t;

static
void imdd_gl3_check_compile_status(GLuint shader)
{
	GLint status = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_TRUE) {
		return;
	}

	GLint max_length = 0;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &max_length);

	if (max_length > 0) {
		char *const info_log = (char *)malloc(max_length);
		GLint actual_length = 0;
		glGetShaderInfoLog(shader, max_length, &actual_length, info_log);
		if (actual_length > 0) {
			fprintf(stderr, "shader error: %s\n", info_log);
		}
		free(info_log);
	}
}

static
void imdd_gl3_check_link_status(GLuint program)
{
	GLint status = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status == GL_TRUE) {
		return;
	}

	GLint max_length = 0;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &max_length);

	if (max_length > 0) {
		char *const info_log = (char *)malloc(max_length);
		GLint actual_length = 0;
		glGetProgramInfoLog(program, max_length, &actual_length, info_log);
		if (actual_length > 0) {
			fprintf(stderr, "program error: %s\n", info_log);
		}
		free(info_log);
	}
}

static
GLuint imdd_gl3_create_shader(GLenum shader_type, char const *src)
{
	GLuint const shader = glCreateShader(shader_type);
	glShaderSource(shader, 1, &src, NULL);
	glCompileShader(shader);
	imdd_gl3_check_compile_status(shader);
	return shader;
}

static
void imdd_gl3_create_program(
	imdd_gl3_program_t *program,
	char const *vs_src,
	char const *fs_src)
{
	GLuint vs = imdd_gl3_create_shader(GL_VERTEX_SHADER, vs_src);
	GLuint fs = imdd_gl3_create_shader(GL_FRAGMENT_SHADER, fs_src);

	GLuint prog = glCreateProgram();
	glAttachShader(prog, vs);
	glAttachShader(prog, fs);
	glLinkProgram(prog);
	imdd_gl3_check_link_status(prog);

	glDeleteShader(vs);
	glDeleteShader(fs);

	program->prog = prog;
	program->proj_from_world_loc = glGetUniformLocation(prog, "g_proj_from_world");
}

static
void imdd_gl3_init_mesh_buffer(imdd_gl3_context_t *ctx, imdd_style_enum_t style)
{
	imdd_gl3_mesh_buffer_t *const mesh_buffer = &ctx->mesh_buffer[style];
	imdd_mesh_layout_t *const mesh_layout = &mesh_buffer->layout;
	uint32_t const vertex_size = (style == IMDD_STYLE_FILLED) ? sizeof(imdd_mesh_filled_vertex_t) : sizeof(imdd_mesh_wire_vertex_t);

	imdd_mesh_layout_init(mesh_layout, style);

	void *const vertices = malloc(mesh_layout->vertex_count*vertex_size);
	uint16_t *const indices = (uint16_t *)malloc(mesh_layout->index_count*sizeof(uint16_t));

	imdd_mesh_layout_write(mesh_layout, vertices, indices);

	glGenBuffers(1, &mesh_buffer->vertex_buf);
	glBindBuffer(GL_ARRAY_BUFFER, mesh_buffer->vertex_buf);
	glBufferData(GL_ARRAY_BUFFER, mesh_layout->vertex_count*vertex_size, vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &mesh_buffer->index_buf);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_buffer->index_buf);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh_layout->index_count*sizeof(uint16_t), indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	free(vertices);
	free(indices);

	mesh_buffer->draw_mode = (style == IMDD_STYLE_FILLED) ? GL_TRIANGLES : GL_LINES;
}

static
void imdd_gl3_init_filled_instance_buffer(imdd_gl3_context_t *ctx)
{
	imdd_gl3_mesh_buffer_t *const mesh_buffer = &ctx->mesh_buffer[IMDD_STYLE_FILLED];
	GLuint *const vertex_array = &ctx->instance_vertex_array[IMDD_STYLE_FILLED];

	imdd_gl3_init_mesh_buffer(ctx, IMDD_STYLE_FILLED);

	glGenVertexArrays(1, vertex_array);
	glBindVertexArray(*vertex_array);
	glBindBuffer(GL_ARRAY_BUFFER, ctx->instance_transform_buf);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(imdd_instance_transform_t), (void *)offsetof(imdd_instance_transform_t, row0));
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(imdd_instance_transform_t), (void *)offsetof(imdd_instance_transform_t, row1));
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(imdd_instance_transform_t), (void *)offsetof(imdd_instance_transform_t, row2));
	glBindBuffer(GL_ARRAY_BUFFER, ctx->instance_color_buf);
	glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(imdd_instance_color_t), 0);
	glBindBuffer(GL_ARRAY_BUFFER, mesh_buffer->vertex_buf);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(imdd_mesh_filled_vertex_t), (void *)offsetof(imdd_mesh_filled_vertex_t, pos));
	glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(imdd_mesh_filled_vertex_t), (void *)offsetof(imdd_mesh_filled_vertex_t, normal));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_buffer->index_buf);
	for (uint32_t i = 0; i < 6; ++i) {
		glEnableVertexAttribArray(i);
	}
	for (uint32_t i = 0; i < 4; ++i) {
		glVertexAttribDivisor(i, 1);
	}
	glBindVertexArray(0);
}

static
void imdd_gl3_init_wire_instance_buffer(imdd_gl3_context_t *ctx)
{
	imdd_gl3_mesh_buffer_t *const mesh_buffer = &ctx->mesh_buffer[IMDD_STYLE_WIRE];
	GLuint *const vertex_array = &ctx->instance_vertex_array[IMDD_STYLE_WIRE];

	imdd_gl3_init_mesh_buffer(ctx, IMDD_STYLE_WIRE);

	glGenVertexArrays(1, vertex_array);
	glBindVertexArray(*vertex_array);
	glBindBuffer(GL_ARRAY_BUFFER, ctx->instance_transform_buf);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(imdd_instance_transform_t), (void *)offsetof(imdd_instance_transform_t, row0));
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(imdd_instance_transform_t), (void *)offsetof(imdd_instance_transform_t, row1));
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(imdd_instance_transform_t), (void *)offsetof(imdd_instance_transform_t, row2));
	glBindBuffer(GL_ARRAY_BUFFER, ctx->instance_color_buf);
	glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(imdd_instance_color_t), 0);
	glBindBuffer(GL_ARRAY_BUFFER, mesh_buffer->vertex_buf);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(imdd_mesh_wire_vertex_t), (void *)offsetof(imdd_mesh_wire_vertex_t, pos));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_buffer->index_buf);
	for (uint32_t i = 0; i < 5; ++i) {
		glEnableVertexAttribArray(i);
	}
	for (uint32_t i = 0; i < 4; ++i) {
		glVertexAttribDivisor(i, 1);
	}
	glBindVertexArray(0);
}

static
void imdd_gl3_init_filled_array_buffer(imdd_gl3_context_t *ctx)
{
	glGenBuffers(1, &ctx->filled_vertex_buf);
	glGenVertexArrays(1, &ctx->filled_vertex_array);

	glBindVertexArray(ctx->filled_vertex_array);
	glBindBuffer(GL_ARRAY_BUFFER, ctx->filled_vertex_buf);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(imdd_array_filled_vertex_t), (void *)0);
	glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(imdd_array_filled_vertex_t), (void *)(3*sizeof(float)));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(imdd_array_filled_vertex_t), (void *)(sizeof(imdd_v4)));
	for (uint32_t i = 0; i < 3; ++i) {
		glEnableVertexAttribArray(i);
	}
	glBindVertexArray(0);
}

static
void imdd_gl3_init_wire_array_buffer(imdd_gl3_context_t *ctx)
{
	glGenBuffers(1, &ctx->wire_vertex_buf);
	glGenVertexArrays(1, &ctx->wire_vertex_array);

	glBindVertexArray(ctx->wire_vertex_array);
	glBindBuffer(GL_ARRAY_BUFFER, ctx->wire_vertex_buf);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(imdd_array_wire_vertex_t), (void *)0);
	glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(imdd_array_wire_vertex_t), (void *)(3*sizeof(float)));
	for (uint32_t i = 0; i < 2; ++i) {
		glEnableVertexAttribArray(i);
	}
	glBindVertexArray(0);
}

#define IMDD_GL3_QUOTE(...) "#version 330\n" #__VA_ARGS__

void imdd_gl3_init(
	imdd_gl3_context_t *ctx,
	uint32_t shape_capacity,
	uint32_t triangle_capacity,
	uint32_t line_capacity)
{
	uint32_t const instance_capacity = shape_capacity;
	uint32_t const filled_vertex_capacity = 3*triangle_capacity;
	uint32_t const wire_vertex_capacity = 2*line_capacity;

	imdd_gl3_create_program(
		&ctx->instance_program[IMDD_STYLE_FILLED],
		IMDD_GL3_QUOTE(
		uniform mat4 g_proj_from_world;
		layout (location = 0) in vec4 a_world0;
		layout (location = 1) in vec4 a_world1;
		layout (location = 2) in vec4 a_world2;
		layout (location = 3) in vec4 a_col;
		layout (location = 4) in vec3 a_pos_ls;
		layout (location = 5) in vec3 a_normal_ls;
		out vec3 v_nvec_ws;
		out vec4 v_col;
		void main(void)
		{
			mat4x3 world_from_local = transpose(mat3x4(a_world0, a_world1, a_world2));
			vec3 pos_ws = world_from_local*vec4(a_pos_ls, 1.0);

			mat3 bivec_world_from_local = inverse(transpose(mat3(world_from_local)));
			vec3 nvec_ws = bivec_world_from_local*a_normal_ls;

			gl_Position = g_proj_from_world*vec4(pos_ws, 1.0);
			v_nvec_ws = nvec_ws;
			v_col = a_col;
		}),
		IMDD_GL3_QUOTE(
		in vec3 v_nvec_ws;
		in vec4 v_col;
		out vec4 o_col;
		void main(void)
		{
			vec3 normal_ws = normalize(v_nvec_ws);
			vec3 light_dir_ws = normalize(vec3(1.0, 2.0, 0.5));

			float n_dot_l = dot(normal_ws, light_dir_ws);
			o_col = vec4(v_col.xyz*(0.5 + 0.45*n_dot_l), v_col.w);
		}));

	imdd_gl3_create_program(
		&ctx->instance_program[IMDD_STYLE_WIRE],
		IMDD_GL3_QUOTE(
		uniform mat4 g_proj_from_world;
		layout (location = 0) in vec4 a_world0;
		layout (location = 1) in vec4 a_world1;
		layout (location = 2) in vec4 a_world2;
		layout (location = 3) in vec4 a_col;
		layout (location = 4) in vec3 a_pos_ls;
		out vec4 v_col;
		void main(void)
		{
			mat4x3 world_from_local = transpose(mat3x4(a_world0, a_world1, a_world2));
			vec3 pos_ws = world_from_local*vec4(a_pos_ls, 1.0);

			gl_Position = g_proj_from_world*vec4(pos_ws, 1.0);
			v_col = a_col;
		}),
		IMDD_GL3_QUOTE(
		in vec4 v_col;
		out vec4 o_col;
		void main(void)
		{
			o_col = v_col;
		}));

	imdd_gl3_create_program(
		&ctx->array_program[IMDD_STYLE_FILLED],
		IMDD_GL3_QUOTE(
		uniform mat4 g_proj_from_world;
		layout (location = 0) in vec3 a_pos_ws;
		layout (location = 1) in vec4 a_col;
		layout (location = 2) in vec3 a_normal_ws;
		out vec3 v_nvec_ws;
		out vec4 v_col;
		void main(void)
		{
			gl_Position = g_proj_from_world*vec4(a_pos_ws, 1.0);
			v_nvec_ws = a_normal_ws;
			v_col = a_col;
		}),
		IMDD_GL3_QUOTE(
		in vec3 v_nvec_ws;
		in vec4 v_col;
		out vec4 o_col;
		void main(void)
		{
			vec3 normal_ws = normalize(v_nvec_ws);
			vec3 light_dir_ws = normalize(vec3(1.0, 2.0, 0.5));

			float n_dot_l = dot(normal_ws, light_dir_ws);
			o_col = vec4(v_col.xyz*(0.5 + 0.45*n_dot_l), v_col.w);
		}));

	imdd_gl3_create_program(
		&ctx->array_program[IMDD_STYLE_WIRE],
		IMDD_GL3_QUOTE(
		uniform mat4 g_proj_from_world;
		layout (location = 0) in vec3 a_pos_ws;
		layout (location = 1) in vec4 a_col;
		out vec4 v_col;
		void main(void)
		{
			gl_Position = g_proj_from_world*vec4(a_pos_ws, 1.0);
			v_col = a_col;
		}),
		IMDD_GL3_QUOTE(
		in vec4 v_col;
		out vec4 o_col;
		void main(void)
		{
			o_col = v_col;
		}));

	glGenBuffers(1, &ctx->instance_transform_buf);
	glGenBuffers(1, &ctx->instance_color_buf);
	imdd_gl3_init_filled_instance_buffer(ctx);
	imdd_gl3_init_wire_instance_buffer(ctx);
	imdd_gl3_init_filled_array_buffer(ctx);
	imdd_gl3_init_wire_array_buffer(ctx);

	ctx->instance_transform_staging = (imdd_instance_transform_t *)malloc(sizeof(imdd_instance_transform_t)*instance_capacity);
	ctx->instance_color_staging = (imdd_instance_color_t *)malloc(sizeof(imdd_instance_color_t)*instance_capacity);
	ctx->instance_capacity = instance_capacity;
	ctx->filled_vertex_staging = (imdd_array_filled_vertex_t *)malloc(sizeof(imdd_array_filled_vertex_t)*filled_vertex_capacity);
	ctx->filled_vertex_capacity = filled_vertex_capacity;
	ctx->wire_vertex_staging = (imdd_array_wire_vertex_t *)malloc(sizeof(imdd_array_wire_vertex_t)*wire_vertex_capacity);
	ctx->wire_vertex_capacity = wire_vertex_capacity;
}

void imdd_gl3_update(
	imdd_gl3_context_t *ctx,
	imdd_shape_store_t const *const *stores,
	uint32_t store_count)
{
	// partition our memory between shapes based on usage, emit all the shapes into it
	uint32_t instance_count = 0;
	uint32_t filled_vertex_count = 0;
	uint32_t wire_vertex_count = 0;
	imdd_emit_shapes(
		stores,
		store_count,
		ctx->instance_transform_staging,
		ctx->instance_color_staging,
		ctx->instance_capacity,
		ctx->instance_batches,
		&instance_count,
		ctx->filled_vertex_staging,
		ctx->filled_vertex_capacity,
		ctx->filled_array_batches,
		&filled_vertex_count,
		ctx->wire_vertex_staging,
		ctx->wire_vertex_capacity,
		ctx->wire_array_batches,
		&wire_vertex_count);

	// upload to GL vertex buffers (consoles would emit directly into graphics memory)
	glBindBuffer(GL_ARRAY_BUFFER, ctx->instance_transform_buf);
	glBufferData(GL_ARRAY_BUFFER, instance_count*sizeof(imdd_instance_transform_t), ctx->instance_transform_staging, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, ctx->instance_color_buf);
	glBufferData(GL_ARRAY_BUFFER, instance_count*sizeof(imdd_instance_color_t), ctx->instance_color_staging, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, ctx->filled_vertex_buf);
	glBufferData(GL_ARRAY_BUFFER, filled_vertex_count*sizeof(imdd_array_filled_vertex_t), ctx->filled_vertex_staging, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, ctx->wire_vertex_buf);
	glBufferData(GL_ARRAY_BUFFER, wire_vertex_count*sizeof(imdd_array_wire_vertex_t), ctx->wire_vertex_staging, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

static
void imdd_gl3_draw_instances(
	imdd_gl3_context_t *ctx,
	imdd_style_enum_t style,
	imdd_blend_enum_t blend,
	imdd_zmode_enum_t zmode)
{
	GLuint const *const vertex_array = &ctx->instance_vertex_array[style];
	glBindVertexArray(*vertex_array);

	for (imdd_mesh_enum_t mesh = (imdd_mesh_enum_t)0; mesh < IMDD_MESH_COUNT; mesh = (imdd_mesh_enum_t)(mesh + 1)) {
		uint32_t const batch_index = imdd_instance_batch_index(mesh, style, blend, zmode);
		imdd_batch_t const *const batch = &ctx->instance_batches[batch_index];
		if (batch->count) {
			imdd_gl3_mesh_buffer_t const *const mesh_buffer = &ctx->mesh_buffer[style];
			imdd_mesh_desc_t const *const mesh_desc = &mesh_buffer->layout.mesh_desc[mesh];
			imdd_mesh_offsets_t const *const mesh_offsets = &mesh_buffer->layout.mesh_offsets[mesh];

			glDrawElementsInstancedBaseInstance(
				mesh_buffer->draw_mode,
				mesh_desc->index_count,
				GL_UNSIGNED_SHORT,
				(void *)(sizeof(uint16_t)*mesh_offsets->index_offset),
				batch->count,
				batch->offset);
		}
	}
}

static
void imdd_gl3_draw_filled_arrays(
	imdd_gl3_context_t *ctx,
	imdd_blend_enum_t blend,
	imdd_zmode_enum_t zmode)
{
	uint32_t const batch_index = imdd_array_batch_index(blend, zmode);
	imdd_batch_t const *const batch = &ctx->filled_array_batches[batch_index];
	if (batch->count) {
		glBindVertexArray(ctx->filled_vertex_array);
		glDrawArrays(GL_TRIANGLES, batch->offset, batch->count);
	}
}

static
void imdd_gl3_draw_wire_arrays(
	imdd_gl3_context_t *ctx,
	imdd_blend_enum_t blend,
	imdd_zmode_enum_t zmode)
{
	uint32_t const batch_index = imdd_array_batch_index(blend, zmode);
	imdd_batch_t const *const batch = &ctx->wire_array_batches[batch_index];
	if (batch->count) {
		glBindVertexArray(ctx->wire_vertex_array);
		glDrawArrays(GL_LINES, batch->offset, batch->count);
	}
}

void imdd_gl3_draw(
	imdd_gl3_context_t *ctx,
	float const *proj_from_world)
{
	// set shader constants
	for (imdd_style_enum_t style = (imdd_style_enum_t)0; style < IMDD_STYLE_COUNT; style = (imdd_style_enum_t)(style + 1)) {
		glUseProgram(ctx->array_program[style].prog);
		glUniformMatrix4fv(ctx->array_program[style].proj_from_world_loc, 1, GL_FALSE, proj_from_world);
		glUseProgram(ctx->instance_program[style].prog);
		glUniformMatrix4fv(ctx->instance_program[style].proj_from_world_loc, 1, GL_FALSE, proj_from_world);
	}

	// emit all draw calls
	for (imdd_zmode_enum_t zmode = (imdd_zmode_enum_t)0; zmode < IMDD_ZMODE_COUNT; zmode = (imdd_zmode_enum_t)(zmode + 1)) {
		if (zmode == IMDD_ZMODE_TEST) {
			glEnable(GL_DEPTH_TEST);
		} else {
			glDisable(GL_DEPTH_TEST);
		}
		for (imdd_blend_enum_t blend = (imdd_blend_enum_t)0; blend < IMDD_BLEND_COUNT; blend = (imdd_blend_enum_t)(blend + 1)) {
			if (blend == IMDD_BLEND_OPAQUE) {
				glDepthMask(GL_TRUE);
				glDisable(GL_BLEND);
			} else {
				glDepthMask(GL_FALSE);
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}
			for (imdd_style_enum_t style = (imdd_style_enum_t)0; style < IMDD_STYLE_COUNT; style = (imdd_style_enum_t)(style + 1)) {
				glEnable(GL_CULL_FACE);
				glUseProgram(ctx->instance_program[style].prog);
				imdd_gl3_draw_instances(ctx, style, blend, zmode);
				glDisable(GL_CULL_FACE);
				glUseProgram(ctx->array_program[style].prog);
				if (style == IMDD_STYLE_FILLED) {
					imdd_gl3_draw_filled_arrays(ctx, blend, zmode);
				} else {
					imdd_gl3_draw_wire_arrays(ctx, blend, zmode);
				}
			}
		}
	}

	// clean up
	glBindVertexArray(0);
	glUseProgram(0);
}

#ifdef __cplusplus
} // extern "C"
#endif
