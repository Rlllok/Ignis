#version 460

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec2 uv;

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
  vec3 texture_normals = normalize(texture(normal_texture, uv*2).rgb*2.0f - 1.0f);
  vec3 specular_light = vec3(dot(-light_direction, norm));
  out_color = (texture(color_texture, uv*2) + vec4(ambient_color, 0.0f))*vec4(specular_light, 1.0f);
  // out_color = vec4(vec3(dot(-light_direction, norm)), 1.0f);
  out_color = vec4(texture_normals, 1.0f);
  out_id = uint(entity_id);
}
