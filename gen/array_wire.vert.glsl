#version 430 core

#ifdef MULTIVIEW
#extension GL_OVR_multiview2 : enable
#define VIEW_COUNT  2
#define VIEW_INDEX  gl_ViewID_OVR
#else
#define VIEW_COUNT  1
#define VIEW_INDEX  0
#endif

layout (location = 0) in vec3 a_pos_ws;
layout (location = 1) in vec4 a_col;

layout(set = 0, binding = 0) uniform common_t {
	mat4 proj_from_world[VIEW_COUNT];
} g_common;

out gl_PerVertex {
	vec4 gl_Position;
};
layout(location = 0) out vec4 v_col;

void main(void)
{
	gl_Position = g_common.proj_from_world[VIEW_INDEX]*vec4(a_pos_ws, 1.0);
	v_col = a_col;
}
