#version 460

vec2 vertecies[4] = {
	{-1.0f,-1.0f},
	{ 1.0f, -1.0f},
	{ 1.0f, 1.0f},
	{-1.0f, 1.0f},
};

int indecies[6] = {0,1,3,1,2,3};

layout(set = 0, binding = 0) uniform GlobalData
{
	mat4 projection;
};

void main()
{
  gl_Position = vec4(vertecies[indecies[gl_VertexIndex]], 0.0f, 1.0f);
}

