#version 460

layout(location = 0) in vec3 position;

// --AlNov: @TODO This should be in fragment shader
layout(binding = 0) uniform UBO
{
  float   time;
  vec2    resolution;
  mat4x4  ortho;
} ubo;

layout(location = 0) out vec2   frag_coordinates;
layout(location = 1) out float  frag_time;
layout(location = 2) out vec2   resolution;

void main()
{
  vec2 coordinates = ((vec2(1.0f) + position.xy) / 2.0f) * ubo.resolution;
  frag_coordinates = coordinates;
  frag_time     = ubo.time;
  resolution    = ubo.resolution;

  gl_Position = ubo.ortho * vec4(vec3(coordinates, 1.0f), 1.0f);
}