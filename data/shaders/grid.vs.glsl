#version 460

vec3 vertecies[4] = {
	{-1.0f, 0.0f, -1.0f},
	{ 1.0f, 0.0f, -1.0f},
	{ 1.0f, 0.0f,  1.0f},
	{-1.0f, 0.0f,  1.0f},
};
int indecies[6] = {0,2,1,2,0,3};

layout(set = 0, binding = 0) uniform GlobaData
{
	mat4 view_matrix;
	mat4 projection_matrix;
};
layout(std140, set = 1, binding = 0) uniform InstanceData
{
	vec3 position;
	float grid_scale;
};

layout(location = 0) out vec3 out_world_position;

void main(void)
{
	vec3 translate = vec3(0.0f,-0.1f, 0.0f);
	out_world_position = (vertecies[indecies[gl_VertexIndex]]*grid_scale) + position;
	gl_Position = projection_matrix*view_matrix*vec4(out_world_position, 1.0f);
}
