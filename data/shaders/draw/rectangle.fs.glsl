#version 460

#extension GL_EXT_buffer_reference    : require
#extension GL_EXT_buffer_reference2   : require

layout(buffer_reference, std430) readonly buffer RectangleDataBuffer {
  mat4x4 projection;
  vec4   position_size;
  vec4   radius;
  vec4   color;
  vec4   border_color;
  float  border_width;
};

layout(push_constant, std430) uniform args {
  RectangleDataBuffer rectangle_data;
};

layout(location = 0) flat in vec4 radius;
layout(location = 1) flat in vec2 half_size;
layout(location = 2)      in vec2 local_xy;

layout(location = 0) out vec4 color_attachment;

float SDF_Rectangle(vec2 position, vec2 half_size, vec4 radius) {
  // check in what corner of rectangle the point is located
  vec2 r = (position.x < 0.0f) ? radius.xy : radius.zw;
  r.x = (position.y < 0.0f) ? r.x : r.y;
  vec2 d = abs(position) - half_size + r.x;
  return length(max(d, 0.0f)) + min(max(d.x, d.y), 0.0f) - r.x;
}

void main() {
  float d = SDF_Rectangle(local_xy - half_size, half_size, radius);
  float aa = fwidth(d);
  float t = 1.0f - smoothstep(0, aa, d);
  
  vec4 color = rectangle_data.color;
  color = mix(color, rectangle_data.border_color, 1.0f - smoothstep(rectangle_data.border_width - aa, rectangle_data.border_width + aa, abs(d)));
  color.a *= t;

  color_attachment = color;
}
