#version 460

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec2 uv;

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
layout(location = 1) out vec3 out_norm;
layout(location = 2) out vec3 out_tangent;
layout(location = 3) out vec2 out_uv;
layout(location = 4) out mat3 out_TBN;

void main(void)
{
  out_position = vec3(instance_matrix*vec4(position, 1.0f));
  out_norm = normalize(vec3(instance_matrix*vec4(norm, 0.0f)));
  out_tangent = tangent;
  out_uv = uv;

  vec3 T = normalize(vec3(instance_matrix*vec4(tangent, 0.0f)));
  vec3 N = out_norm;
  vec3 B = cross(N, T);
  mat3 TBN = mat3(T, B, N);
  out_TBN = TBN;

  gl_Position = projection_matrix*view_matrix*vec4(out_position, 1.0f);
}
