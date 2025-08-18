#version 460

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec4 out_color;

void main(void)
{
  out_color = vec4(step(0.9f, abs(uv*2.0f - 1.0f)), 0.0f, 1.0f);
}
