#version 460

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

layout(binding = 0) uniform MVP
{
  vec3 color;
  vec3 center_position;
  mat4 view;
} mvp;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec3 fragPosition;
layout(location = 3) out vec2 fragUV;

void main()
{
  fragColor    = mvp.color;
  fragNormal   = normal;
  fragPosition = position + mvp.center_position;
  fragUV       = uv;
  gl_Position  = vec4(fragPosition, 1.0f);
}
