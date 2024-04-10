#version 460

layout(location = 0) in vec3 position;

layout(binding = 0) uniform MVP
{
  vec3 color;
  vec3 center_position;
} mvp;

vec3 positions[] = {
  vec3(0.0f, -0.5f, 0.0f),
  vec3(0.5f, 0.5f, 0.0f),
  vec3(-0.5f, 0.5f, 0.0f),
};  

layout(location = 0) out vec3 fragColor;

void main()
{
  fragColor = mvp.color;
  gl_Position = vec4(position + mvp.center_position, 1.0f);
}
