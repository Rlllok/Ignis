#version 460

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;

layout(set = 0, binding = 0) uniform GlobalData
{
  mat4 view_matrix;
  mat4 projection_matrix;
};

layout(location = 0) out vec3 out_position;
layout(location = 1) out vec2 out_uv;

void main(void)
{
  out_position = position;
  out_uv = uv;
  gl_Position = projection_matrix*view_matrix*vec4(position, 1.0f);
}
