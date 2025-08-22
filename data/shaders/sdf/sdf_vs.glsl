#version 460

vec3 vertecies[4] = {
	{-1.0f,-1.0f, 0.0f},
	{ 1.0f,-1.0f, 0.0f},
	{ 1.0f, 1.0f, 0.0f},
	{-1.0f, 1.0f, 0.0f},
};
int indecies[6] = {0,1,2,2,3,0};

layout(set = 0, binding = 0) uniform GlobalData
{
  mat4x4 projection;
};

layout(location = 0) out vec2 uv;

void main()
{
  vec3 position = vertecies[indecies[gl_VertexIndex]];
  uv = position.xy;

  gl_Position = vec4(position, 1.0f);
}
