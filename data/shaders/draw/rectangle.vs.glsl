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
  vec4   color;
};

layout(push_constant, std430) uniform args {
  RectangleDataBuffer rectangle_data;
};

void main() {
  vec4 offset = vec4(rectangle_data.position_size.xy, 0.0f, 0.0f);
  vec4 scale = vec4(rectangle_data.position_size.zw, 0.0f, 1.0f);
  vec4 vertex = vertecies[indecies[gl_VertexIndex]]*scale + offset;
  gl_Position = rectangle_data.projection*vertex;
}
