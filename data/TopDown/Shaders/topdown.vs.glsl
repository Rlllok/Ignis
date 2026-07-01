#version 460

#extension GL_EXT_buffer_reference    : require
#extension GL_EXT_buffer_reference2   : require

layout(location = 0) in vec3  position;
layout(location = 1) in vec3  normal;
layout(location = 2) in vec3  tangent;
layout(location = 3) in vec2  uv;
layout(location = 4) in ivec4 joint_ids;
layout(location = 5) in vec4  joint_weights;

struct SceneData {
  vec3 light_direction;
  vec3 light_color;
};

layout(buffer_reference, std430) readonly buffer SceneDataBuffer {
  SceneData data;
};

struct Material {
  vec3 color;
};

struct ObjectData {
  mat4x4           transform;
  mat4x4           camera_transform;
  Material         material;
};

layout(buffer_reference, std430) readonly buffer ObjectDataBuffer {
  ObjectData data;
};

layout(push_constant, std430) uniform args {
  SceneDataBuffer  scene_data;
  ObjectDataBuffer objects_data;
};

layout(location = 0)      out vec3 out_position;
layout(location = 1)      out vec3 out_normal;
layout(location = 2) flat out vec3 out_color;

void main() {
  ObjectData current_object = objects_data[gl_InstanceIndex].data;

  vec4 world_position = current_object.transform*vec4(position, 1.0f);

  out_position = vec3(world_position);
  // --AlNov: @TODO Normal Matrix should be calculated once on the CPU as inverse is costly operation
  out_normal = transpose(inverse(mat3(current_object.transform)))*normal;
  out_color = current_object.material.color;
  gl_Position = current_object.camera_transform*world_position;
}
