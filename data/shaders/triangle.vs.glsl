#version 460

layout(location = 0) in vec3 position;

vec3 colors[3] = {
  vec3(1.0f, 0.0f, 0.0f),
  vec3(0.0f, 1.0f, 0.0f),
  vec3(0.0f, 0.0f, 1.0f),
};

layout(location = 0) out vec3 out_color;

void main() {
  out_color = colors[gl_VertexIndex];

	gl_Position = vec4(position, 1.0f);
}
