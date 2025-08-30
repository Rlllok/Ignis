#version 460

vec2 vertecies[4] = {
	{ 0.0f, 0.0f},
	{ 1.0f, 0.0f},
	{ 1.0f, 1.0f},
	{ 0.0f, 1.0f},
};

int indecies[6] = {0,1,3,1,2,3};

layout(set = 0, binding = 0) uniform GlobalData
{
	mat4 projection;
};

layout(set = 1, binding = 0) uniform InstanceData
{
  vec2 position;
  vec2 size;
};

layout(location = 0) out vec2 out_position;
layout(location = 1) out vec2 out_size;

void main()
{
  out_position = vertecies[indecies[gl_VertexIndex]]*size;
  out_size = size;

  vec2 screen_position = (vertecies[indecies[gl_VertexIndex]]*size) + position;
  gl_Position = projection*vec4(screen_position.xy, 0.0f, 1.0f);
}
