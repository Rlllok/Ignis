#version 460

layout(location = 0) in vec3 position;

// --AlNov: @TODO This should be in fragment shader
layout(binding = 0) uniform UBO
{
  mat4x4  projection;
  vec2    resolution;
  vec2    position;
  vec2    size;
  float   is_box;
} ubo;

layout(location = 0) out vec2   pixel;
layout(location = 1) out vec2   uv;
layout(location = 2) out vec2   resolution;
layout(location = 3) out vec2   center_position;
layout(location = 4) out vec2   size;
layout(location = 5) out float  is_box;

void main()
{
  vec2 coordinates = ((vec2(1.0f) + position.xy) / 2.0f) * ubo.resolution;
  pixel           = coordinates;
  uv              = pixel / ubo.resolution;
  resolution      = ubo.resolution;
  center_position = ubo.position;
  size            = ubo.size;
  is_box          = ubo.is_box;

  gl_Position = ubo.projection * vec4(vec3(coordinates, 1.0f), 1.0f);
}