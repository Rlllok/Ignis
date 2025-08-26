#version 460
layout(location = 0) in vec2 position;
layout(location = 1) in vec2 uv;

layout(set = 0, binding = 0) uniform GlobalData
{
  mat4 projection;
  vec4 text_color;
};

layout(location = 0) out vec2 out_uv;
layout(location = 1) flat out vec4 out_color;

void main(void)
{
  out_color = text_color;
  out_uv = uv;

  gl_Position = projection*vec4(position, 0.0f, 1.0f);
}
