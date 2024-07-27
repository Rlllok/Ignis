#version 460

layout(location = 0) in vec3 frag_color;
layout(location = 1) in vec3 frag_normal;
layout(location = 2) in vec3 frag_position;
layout(location = 3) in vec2 frag_uv;

layout(binding = 1) uniform samplerCube cubemap_sampler;

layout(location = 0) out vec4 out_color;

void main()
{
  // out_color = texture(cubemap_sampler, frag_position);
  out_color = vec4(1.0f);
}