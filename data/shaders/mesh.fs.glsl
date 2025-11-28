#version 460

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec2 uv;
layout(location = 4) in vec3 weight_color;
layout(location = 5) in mat3 TBN;

layout(set = 3, binding = 0) uniform InstanceData
{
  vec3 camera_position;
  vec3 ambient_color;
  float smoothness;
  vec3 light_direction;
  float entity_id;
};

layout(set = 3, binding = 1) uniform sampler2D color_texture;
layout(set = 3, binding = 2) uniform sampler2D normal_texture;

layout(location = 0) out vec4 out_color;
layout(location = 1) out uint out_id;

void main(void)
{
  vec3 texture_normals = TBN*normalize(texture(normal_texture, uv).rgb*2.0f - 1.0f);

  vec3 ambient_light = ambient_color;
  vec3 diffuse_light = vec3(max(dot(-light_direction, texture_normals), 0.0f));

  vec3 view_direction = normalize(camera_position - position);
  vec3 reflection_direction = reflect(light_direction, texture_normals);
  vec3 specular_light = vec3(0.7f)*0.35f*pow(max(dot(view_direction, reflection_direction), 0.0f), log2(smoothness*10 + 1));

  out_color = texture(color_texture, uv)*vec4(ambient_color + diffuse_light + specular_light, 1.0f);
  // out_color = vec4(weight_color, 1.0f);
  out_id = uint(entity_id);
}
