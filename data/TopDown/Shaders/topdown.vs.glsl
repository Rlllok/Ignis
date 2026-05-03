#version 460

#extension GL_EXT_buffer_reference    : require
#extension GL_EXT_buffer_reference2   : require

layout(std430, buffer_reference) readonly buffer VertexDataBuffer {
  vec3 position;
  vec3 normal;
  vec3 tangent;
  vec2 uv;
  vec4 joint_ids;
  vec4 joint_weights;
};

struct Material {
  vec3 color;
};

layout(buffer_reference) readonly buffer ObjectDataBuffer {
  mat4x4           transform;
  mat4x4           camera_transform;
  Material         material;
  VertexDataBuffer vertex_ptr;
};

layout(set = 0, binding = 0) uniform GlobalData {
  ObjectDataBuffer data_buffer;
};

layout(location = 0)      out vec3 out_position;
layout(location = 1)      out vec3 out_normal;
layout(location = 2) flat out vec3 out_color;

void main() {
  int object_index = gl_InstanceIndex;

  VertexDataBuffer vertecies = data_buffer[object_index].vertex_ptr;

  vec4 world_position = data_buffer[object_index].transform*vec4(vertecies[gl_VertexIndex].position, 1.0f);

  out_position = vec3(world_position);
  out_normal = vertecies[gl_VertexIndex].normal;
  out_color = data_buffer[object_index].material.color;
  gl_Position = data_buffer[object_index].camera_transform*world_position;
}
