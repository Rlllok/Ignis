#version 460

layout(set = 3, binding = 0) uniform InstanceData
{
  vec4 color;
  vec4 radius;
};

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 size;

layout(location = 0) out vec4 out_color;

void main()
{
  vec4 clamped_radius = clamp(radius, 0.0f, min(size.x*0.5, size.y*0.5f));
  vec2 new_position = position - 0.5f*size;
  vec2 r_xy = (new_position.x < 0.0f) ? clamped_radius.xz : clamped_radius.yw;
  float radius_value = (new_position.y < 0.0f) ? r_xy.x : r_xy.y;
  vec2 dist = abs(new_position) - 0.5f*size + radius_value;
  float rectangle = length(max(dist, 0.0f)) + min(max(dist.x, dist.y), 0.0f) - radius_value;
  rectangle = smoothstep(3.0f, 0.0f, rectangle+3.0f);

  out_color = color;
  out_color.a *= rectangle;
}
