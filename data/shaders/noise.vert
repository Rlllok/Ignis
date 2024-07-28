#version 460

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec3 fragPosition;
layout(location = 3) out vec2 frag_uv;

void main()
{
  frag_uv = uv;

  gl_Position = vec4(position, 1.0f);
}