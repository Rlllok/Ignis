#version 460

layout(location = 0) in vec3 position;

layout(set = 1, binding = 0) uniform InstanceData {
  mat4 transform;
};

void main() {
  gl_Position = transform*vec4(position, 1.0f);
};
