#version 460

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

struct Material {
  vec3 color;
};

layout(set = 1, binding = 0) uniform InstanceData {
  mat4     transform;
  mat4     camera_transform;
  Material material;
};

layout(location = 0)      out vec3 out_position;
layout(location = 1)      out vec3 out_normal;
layout(location = 2) flat out vec3 out_color;

void main() {
  vec4 world_position = transform*vec4(position, 1.0f);

  out_position = vec3(world_position);
  out_normal = normal;
  out_color = material.color;
  gl_Position = camera_transform*world_position;
}
