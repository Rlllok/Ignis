#version 460

#extension GL_EXT_buffer_reference    : require
#extension GL_EXT_buffer_reference2   : require

layout(buffer_reference, std430) readonly buffer PillarsDataBuffer {
  vec3   player_position;
  vec3   camera_position;
  mat4x4 camera_inverse;
  vec4   color;
  vec2   resolution;
};

layout(push_constant, std430) uniform args {
  PillarsDataBuffer pillars_data;
};

layout(location = 0) out vec4 color_attachment;

void main() {
  color_attachment = pillars_data.color;
}
