#version 430 core

#ifdef MULTIVIEW
#extension GL_OVR_multiview2 : enable
#define VIEW_COUNT  2
#define VIEW_INDEX  gl_ViewID_OVR
#else
#define VIEW_COUNT  1
#define VIEW_INDEX  0
#endif

layout (location = 0) in vec4 a_world0;
layout (location = 1) in vec4 a_world1;
layout (location = 2) in vec4 a_world2;
layout (location = 3) in vec4 a_col;
layout (location = 4) in vec3 a_pos_ls;
layout (location = 5) in vec3 a_normal_ls;

layout(set = 0, binding = 0) uniform common_t {
	mat4 proj_from_world[VIEW_COUNT];
} g_common;

out gl_PerVertex {
	vec4 gl_Position;
};
layout(location = 0) out vec3 v_nvec_ws;
layout(location = 1) out vec4 v_col;

void main(void)
{
	mat4x3 world_from_local = transpose(mat3x4(a_world0, a_world1, a_world2));
	vec3 pos_ws = world_from_local*vec4(a_pos_ls, 1.0);

	mat3 bivec_world_from_local = inverse(transpose(mat3(world_from_local)));
	vec3 nvec_ws = bivec_world_from_local*a_normal_ls;

	gl_Position = g_common.proj_from_world[VIEW_INDEX]*vec4(pos_ws, 1.0);
	v_nvec_ws = nvec_ws;
	v_col = a_col;
}
