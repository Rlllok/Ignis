#version 460

layout(location = 0) in vec2 uv;

layout(binding = 1) uniform sampler2D texture_sampler;

layout(location = 0) out vec4 out_color;

void main()
{
  vec2 transformed_uv = uv;
  vec4 texture_color = texture(texture_sampler, transformed_uv);

  out_color = texture_color;
}
