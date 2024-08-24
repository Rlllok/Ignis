#version 460

layout(location = 0) in vec3 position;

// --AlNov: @TODO This should be in fragment shader
layout(binding = 0) uniform UBO
{
  float   time;
  vec2    resolution;
  mat4x4  ortho;
  vec2    mouse_position;
} ubo;

layout(location = 0) out vec2   frag_coordinates;
layout(location = 1) out float  frag_time;
layout(location = 2) out vec2   resolution;
layout(location = 3) out vec2   mouse_position;

void main()
{
  vec2 coordinates = ((vec2(1.0f) + position.xy) / 2.0f) * ubo.resolution;
  frag_coordinates = coordinates;
  frag_time        = ubo.time;
  resolution       = ubo.resolution;
  mouse_position   = ubo.mouse_position;
  mouse_position.y = resolution.y - mouse_position.y;

  gl_Position = ubo.ortho * vec4(vec3(coordinates, 1.0f), 1.0f);
}