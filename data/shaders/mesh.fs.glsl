#version 460

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec2 uv;
layout(location = 4) in mat3 TBN;

layout(set = 3, binding = 0) uniform InstanceData
{
  vec3 ambient_color;
  float entity_id;
  vec3 light_direction;
};

layout(set = 3, binding = 1) uniform sampler2D color_texture;
layout(set = 3, binding = 2) uniform sampler2D normal_texture;

layout(location = 0) out vec4 out_color;
layout(location = 1) out uint out_id;

void main(void)
{
  vec3 ambient_light = ambient_color;
  vec3 texture_normals = TBN*normalize(texture(normal_texture, uv).rgb*2.0f - 1.0f);
  vec3 specular_light = vec3(max(dot(-light_direction, texture_normals), 0.0f));
  out_color = texture(color_texture, uv)*vec4(ambient_color + specular_light, 1.0f);
  // out_color = vec4(vec3(dot(-light_direction, norm)), 1.0f);
  // out_color = vec4(texture_normals, 1.0f);
  // out_color = vec4(TBN*texture_normals, 1.0f);
  out_id = uint(entity_id);
}
