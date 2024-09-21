#version 460

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 NOT_IN_USE;
layout(location = 2) in vec2 a_uv;

// --AlNov: @TODO This should be in fragment shader
layout(binding = 0) uniform UBO
{
  mat4x4  projection;
  vec2    position;
  vec2    min_p;
  vec2    max_p;
  vec2    texture_size;
} ubo;

layout(location = 0) out vec2 out_uv;

void main()
{
  const vec2 sprite_uv = a_uv / ubo.texture_size;
  const vec2 size      = ubo.max_p - ubo.min_p;

  vec3 vertex_position = a_position * vec3(size, 0.0f) + vec3(ubo.position, 0.0f);
  gl_Position = ubo.projection * vec4(vertex_position, 1.0f);
  gl_Position.z = 0.0f;

  out_uv = sprite_uv;
}
