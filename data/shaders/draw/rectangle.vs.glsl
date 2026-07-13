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
  vec4   radius;
  vec4   top_left_color;
  vec4   top_right_color;
  vec4   bottom_right_color;
  vec4   bottom_left_color;
  vec4   border_color;
  float  border_width;
};

layout(push_constant, std430) uniform args {
  RectangleDataBuffer rectangle_data;
};

layout(location = 0) flat out vec4 out_radius;
layout(location = 1) flat out vec2 out_half_size;
layout(location = 2)      out vec2 out_local_xy;

void main() {
  vec4 offset = vec4(rectangle_data.position_size.xy, 0.0f, 0.0f);
  float antialising_padding = 5.0f;
  vec4 scale = vec4(rectangle_data.position_size.zw + vec2(antialising_padding), 0.0f, 1.0f);
  vec4 vertex = vertecies[indecies[gl_VertexIndex]]*scale + offset;

  out_radius = rectangle_data.radius;
  out_half_size = rectangle_data.position_size.zw/2.0f;
  out_local_xy = vertex.xy - rectangle_data.position_size.xy;
  gl_Position = rectangle_data.projection*vertex;
}
