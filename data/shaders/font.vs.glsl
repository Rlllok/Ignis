#version 460

#if 0 
vec2 vertecies[4] = {
	{-1.0f,-1.0f},
	{ 1.0f,-1.0f},
	{ 1.0f, 1.0f},
	{-1.0f, 1.0f},
};
#endif

vec2 vertecies[4] = {
	{ 0.0f, 0.0f},
	{ 1.0f, 0.0f},
	{ 1.0f, 1.0f},
	{ 0.0f, 1.0f},
};

vec2 uvs[4] = {
  {0.0f, 0.0f},
  {1.0f, 0.0f},
  {1.0f, 1.0f},
  {0.0f, 1.0f},
};

int indecies[6] = {0,2,1,2,0,3};

layout(set = 0, binding = 0) uniform GlobalData
{
  mat4 projection;
};

layout(set = 1, binding = 0) uniform InstanceData
{
  vec2 glyph_position;
  vec2 glyph_size;
  vec2 glyph_uv_offset;
  vec2 glyph_uv_size;
};

layout(location = 0) out vec2 uv;

void main(void)
{
  uv = uvs[indecies[gl_VertexIndex]]*glyph_uv_size + glyph_uv_offset;
  vec4 position = projection*vec4(glyph_position + glyph_size*vertecies[indecies[gl_VertexIndex]], 0.0f, 1.0f);
  gl_Position = position;
}
