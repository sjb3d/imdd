#version 430 core

layout(location = 0) in vec4 v_col;

layout(location = 0) out vec4 o_col;

void main(void)
{
	o_col = v_col;
}
