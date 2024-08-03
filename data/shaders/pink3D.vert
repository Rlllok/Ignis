#version 460

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

layout(binding = 0) uniform MVP
{
  vec3 color;
  mat4 view;
  mat4 translation;
} mvp;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec3 fragPosition;

void main()
{
  fragColor    = mvp.color;
  fragNormal   = normal;
  fragPosition = vec3(mvp.translation * vec4(position, 1.0f));
  gl_Position  = mvp.view * mvp.translation * vec4(position, 1.0f);
}
