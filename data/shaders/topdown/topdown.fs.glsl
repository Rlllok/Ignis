#version 460

layout(location = 0) in vec3 position;

layout(location = 0) out vec4 color_attachment;

void main() {
  color_attachment = vec4(position, 1.0f);
}
