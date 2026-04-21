#version 460

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

layout(set = 1, binding = 0) uniform InstanceData {
  mat4 mvp;
};

layout(location = 0) out vec3 out_position;
layout(location = 1) out vec3 out_normal;

void main() {
  out_position = position;
  out_normal = normal;
  gl_Position = mvp*vec4(position, 1.0f);
}
