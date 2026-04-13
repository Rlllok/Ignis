#version 460

layout(location = 0) in vec3 position;

layout(set = 1, binding = 0) uniform InstanceData {
  mat4 mvp;
};

layout(location = 0) out vec3 out_position;

void main() {
  out_position = position;
  gl_Position = mvp*vec4(position, 1.0f);
}
