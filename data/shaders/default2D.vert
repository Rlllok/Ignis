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
  vec2 viewport = vec2(1280.0f, 720.0f);
  fragColor = mvp.color;
  vec3 position = position;
  position.x = position.x / 1280.0f * 2 - 1;
  position.y = position.y / 720.0f * 2 - 1;
  gl_Position = vec4(position, 1.0f);
}
