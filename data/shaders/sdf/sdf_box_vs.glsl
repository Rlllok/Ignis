#version 460

layout(location = 0) in vec3 a_position;

// --AlNov: @TODO This should be in fragment shader
layout(binding = 0) uniform UBO
{
  mat4x4  projection;
  vec2    position;
  vec2    size;
  vec3    color;
} ubo;

layout(location = 0) out vec2   uv;
layout(location = 1) out vec3   color;

void main()
{
  uv    = a_position.xy;
  color = ubo.color;

  vec3 vertex_position = a_position * vec3(ubo.size, 0.0f) + vec3(ubo.position, 0.0f);
  vertex_position += vec3(ubo.size, 0.0f); // --AlNov: Anchor in upper-left corner
  gl_Position = ubo.projection * vec4(vertex_position, 1.0f);
  gl_Position.z = 0.0f;
}
