#version 460

#extension GL_EXT_buffer_reference    : require
#extension GL_EXT_buffer_reference2   : require

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 color;

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

layout(location = 0) out vec4 color_attachment;

void main() {
  SceneData scene = scene_data[0].data;

  float diffuse_factor = max(dot(normal, -scene.light_direction), 0.0f);

  color_attachment = vec4(diffuse_factor*color, 1.0f);
}
