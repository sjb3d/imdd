#include "gl_core_3_3.h"
#include <GLFW/glfw3.h>
#define IMDD_IMPLEMENTATION
#include "imdd.h"
#include "imdd_draw_gl3.h"
#include <stdio.h>
#include <stdlib.h>
#include "example_common.h"

#define VERIFY(STMT)									\
	do { 												\
		if (!(STMT)) {									\
			fprintf(stderr, "failed: %s\n", #STMT);		\
			exit(-1);									\
		}												\
	} while (0)

void error_callback(int error, char const *description)
{
	UNUSED(error);
	fprintf(stderr, "GLFW: %s\n", description);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	UNUSED(scancode);
	UNUSED(mods);
	if (action == GLFW_PRESS) {
		switch (key) {
			case GLFW_KEY_ESCAPE:
				glfwSetWindowShouldClose(window, GL_TRUE);;
				break;
		}
	}
}

int main(int argc, char *argv[])
{
	UNUSED(argc);
	UNUSED(argv);

	glfwSetErrorCallback(&error_callback);
	VERIFY(glfwInit());

	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_SRGB_CAPABLE, GL_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_DEPTH_BITS, 16);
	glfwWindowHint(GLFW_STENCIL_BITS, 0);
	GLFWwindow *const window = glfwCreateWindow(960, 544, "imdd example", NULL, NULL);
	VERIFY(window);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	glfwSetKeyCallback(window, &key_callback);

	VERIFY(ogl_LoadFunctions() == ogl_LOAD_SUCCEEDED);

	uint32_t const shape_count = 64*1024;
	uint32_t const shape_mem_size = IMDD_APPROX_SHAPE_SIZE_IN_BYTES*shape_count;
	imdd_shape_store_t *const store = imdd_init(malloc(shape_mem_size), shape_mem_size);

	imdd_gl3_context_t ctx;
	imdd_gl3_init(&ctx, shape_count, shape_count, shape_count);

	float angle = 0.f;
	while (!glfwWindowShouldClose(window)) {
		// reset debug draw and emit test shapes
		imdd_reset(store);
		imdd_example_test(store);

		// set current window size
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
		glEnable(GL_FRAMEBUFFER_SRGB);

		// clear back buffer
		glClearColor(.1f, .1f, .1f, 1.f);
		glDepthMask(GL_TRUE);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// spin the camera
		angle += .5f*PI/60.f;
		mat4 proj_from_world;
		mat4_identity(&proj_from_world);
		mat4_rotate_y(&proj_from_world, angle);
		mat4_rotate_x(&proj_from_world, PI/8.f);
		mat4_translation(&proj_from_world, 0.f, 0.f, -22.f);
		mat4_perspective_gl(&proj_from_world, PI/8.f, (float)width/(float)height, .1f, 100.f);

		// emit debug draw
		imdd_shape_store_t const *draw_store = store;
		imdd_gl3_update(&ctx, &draw_store, 1);
		imdd_gl3_draw(&ctx, proj_from_world.m[0]);

		// flip
		glDisable(GL_FRAMEBUFFER_SRGB);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
