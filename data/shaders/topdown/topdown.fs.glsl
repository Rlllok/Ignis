#version 460

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 color;

struct Light {
  vec3 direction;
  vec3 color;
};

layout(set = 2, binding = 0) uniform GlobalData {
  Light light;
};

layout(location = 0) out vec4 color_attachment;

void main() {
  vec3 ambient = 0.1f*light.color;

  vec3 n = normalize(normal);
  vec3 diffuse = max(dot(n, -light.direction), 0.0f)*light.color;

  color_attachment = vec4((ambient + diffuse)*color, 1.0f);
}
