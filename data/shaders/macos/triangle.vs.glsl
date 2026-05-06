#version 460

#extension GL_EXT_buffer_reference    : require
#extension GL_EXT_buffer_reference2   : require

struct Material {
  vec4 color;
};

layout(std430, buffer_reference) readonly buffer MaterialBuffer {
  Material material;
};

struct EntityData {
  vec4 translation;
};

layout(std430, buffer_reference) readonly buffer EntityDataBuffer {
  EntityData entity_data;
};

layout(location = 0) in vec3 position;

layout(push_constant, std430) uniform args {
  MaterialBuffer   materials;
  EntityDataBuffer entity_datas;
};

layout (location = 0) out vec4 out_color;

void main() {
  gl_Position = vec4(position, 1.0f) + entity_datas[gl_InstanceIndex].entity_data.translation;
  out_color = materials[gl_InstanceIndex].material.color;
};
