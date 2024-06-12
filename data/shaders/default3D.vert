#version 460

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

layout(binding = 0) uniform MVP
{
  vec3 color;
  vec3 center_position;
  mat4 view;
} mvp;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec3 fragPosition;

void main()
{
  fragColor    = mvp.color;
  fragNormal   = normal;
  fragPosition = position;
  gl_Position  = vec4(position, 1.0f);
}
