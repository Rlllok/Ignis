#version 460

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 color;

layout(location = 0) out vec4 color_attachment;

void main() {
  color_attachment = vec4(color, 1.0f);
}
