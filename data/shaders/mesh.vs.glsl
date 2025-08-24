#version 460

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;

layout(set = 0, binding = 0) uniform GlobalData
{
  mat4 view_matrix;
  mat4 projection_matrix;
};

layout(set = 1, binding = 0) uniform InstanceData
{
  mat4 instance_matrix;
  // vec3 translate;
};

layout(location = 0) out vec3 out_position;
layout(location = 1) out vec2 out_uv;

void main(void)
{
  out_position = vec3(instance_matrix*vec4(position, 1.0f));
  out_uv = uv;
  gl_Position = projection_matrix*view_matrix*vec4(out_position, 1.0f);
}
