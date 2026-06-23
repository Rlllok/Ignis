#version 460

#extension GL_EXT_buffer_reference    : require
#extension GL_EXT_buffer_reference2   : require

const vec4 vertecies[] = {
  vec4(0.0f, 1.0f, 0.0f, 1.0f),
  vec4(1.0f, 1.0f, 0.0f, 1.0f),
  vec4(1.0f, 0.0f, 0.0f, 1.0f),
  vec4(0.0f, 0.0f, 0.0f, 1.0f),
};

const uint indecies[] = {
  0, 1, 3,
  1, 2, 3
};

layout(buffer_reference, std430) readonly buffer RectangleDataBuffer {
  mat4x4 projection;
  vec4   position_size;
  vec3   hsv;
};

layout(push_constant, std430) uniform args {
  RectangleDataBuffer rectangle_data;
};

layout(location = 0) out flat vec2 half_size;
layout(location = 1) out      vec2 local_xy;

void main() {
  float antialising_padding = 5.0f;
  vec4 vertex_position = vertecies[indecies[gl_VertexIndex]]*vec4(rectangle_data.position_size.z + antialising_padding, rectangle_data.position_size.w + antialising_padding, 0.0f, 1.0f) + vec4(rectangle_data.position_size.x, rectangle_data.position_size.y, 0.0f, 0.0f);

  gl_Position = rectangle_data.projection*vertex_position;
  half_size  = rectangle_data.position_size.zw/2.0f;
  local_xy = vertex_position.xy - rectangle_data.position_size.xy;
}
