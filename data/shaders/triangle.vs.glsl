#version 460

layout(set = 0, binding = 0) uniform GlobalData
{
	mat4 scale_matrix;
	mat4 transpose_matrix;
};

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;

layout(location = 0) out vec4 out_color;

void main() {
	out_color = color;
	gl_Position = transpose_matrix*scale_matrix*vec4(position, 1.0f);
}
