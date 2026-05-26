#version 460

#extension GL_EXT_buffer_reference    : require
#extension GL_EXT_buffer_reference2   : require

layout(buffer_reference, std430) readonly buffer RectangleDataBuffer {
  mat4x4 projection;
  vec4 position_size;
  vec4 color;
};

layout(push_constant, std430) uniform args {
  RectangleDataBuffer rectangle_data;
};

layout(location = 0) out vec4 color_attachment;

void main() {
  color_attachment = rectangle_data.color;
}
