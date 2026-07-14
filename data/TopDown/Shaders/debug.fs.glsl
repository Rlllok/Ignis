#version 460

#extension GL_EXT_buffer_reference    : require
#extension GL_EXT_buffer_reference2   : require

layout(buffer_reference, std430) readonly buffer BoundingBoxBuffer {
  mat4x4 transform;
  mat4x4 camera_transform;
  vec4   rgba;
};

layout(push_constant, std430) uniform args {
  BoundingBoxBuffer box_data;
};

layout(location = 0) out vec4 color_attachment;

void main() {
  color_attachment = box_data.rgba;
}
