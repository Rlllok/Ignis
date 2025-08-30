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

void main()
{
  vec2 screen_position = (vertecies[indecies[gl_VertexIndex]]*size) + position;
  gl_Position = projection*vec4(screen_position.xy, 0.0f, 1.0f);
}
