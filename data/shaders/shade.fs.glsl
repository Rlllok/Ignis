#version 460

layout(location = 0) in in_value {
  vec2 uv;
};

layout(location = 0) out vec4 out_color;

void main() {
  out_color = vec4(uv, 0.0f, 1.0f);
}
