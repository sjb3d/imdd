#version 430 core

layout(location = 0) in vec3 v_nvec_ws;
layout(location = 1) in vec4 v_col;

layout(location = 0) out vec4 o_col;

void main(void)
{
	vec3 normal_ws = normalize(v_nvec_ws);
	vec3 light_dir_ws = normalize(vec3(1.0, 2.0, 0.5));

	float n_dot_l = dot(normal_ws, light_dir_ws);
	o_col = vec4(v_col.xyz*(0.5 + 0.45*n_dot_l), v_col.w);
}
