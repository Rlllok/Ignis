#version 460

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec2 uv;
layout(location = 4) in ivec4 joint_ids;
layout(location = 5) in vec4 joint_weights;

layout(set = 0, binding = 0) uniform GlobalData
{
  mat4 view_matrix;
  mat4 projection_matrix;
};

const int MAX_BONES = 64;

layout(set = 1, binding = 0) uniform InstanceData
{
  mat4 instance_matrix;
  mat4 joint_transform[MAX_BONES];
  // vec3 translate;
};

layout(location = 0) out vec3 out_position;
layout(location = 1) out vec3 out_norm;
layout(location = 2) out vec3 out_tangent;
layout(location = 3) out vec2 out_uv;
layout(location = 4) out vec3 out_weight_color;
layout(location = 5) out mat3 out_TBN;

void main(void)
{
  mat4 total_skin_transform = joint_weights.x*joint_transform[joint_ids.x];
  total_skin_transform += joint_weights.y*joint_transform[joint_ids.y];
  total_skin_transform += joint_weights.z*joint_transform[joint_ids.z];
  total_skin_transform += joint_weights.w*joint_transform[joint_ids.w];

  total_skin_transform = mat4(1.0f);

  out_position = vec3(total_skin_transform*vec4(position, 1.0f));
  out_norm = normalize(vec3(instance_matrix*vec4(norm, 0.0f)));
  out_tangent = tangent;
  out_uv = uv;
  out_weight_color = vec3(0.0f, 0.0f, 1.0f);
  
  if (joint_ids.x == 1)
  {
    out_weight_color = mix(out_weight_color, vec3(1.0f, 0.0f, 0.0f), joint_weights.x);
  }

  vec3 T = normalize(vec3(instance_matrix*vec4(tangent, 0.0f)));
  vec3 N = out_norm;
  vec3 B = cross(N, T);
  mat3 TBN = mat3(T, B, N);
  out_TBN = TBN;

  gl_Position = projection_matrix*view_matrix*instance_matrix*total_skin_transform*vec4(position, 1.0f);
}
