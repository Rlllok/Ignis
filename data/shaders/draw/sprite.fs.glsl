#version 460

#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_buffer_reference     : require
#extension GL_EXT_buffer_reference2    : require

layout(buffer_reference, std430) readonly buffer SpriteDataBuffer {
  mat4x4 projection;
  mat4x4 transform;
  vec4   part_position_size;
  vec4   tint_color;
  uint   texture_index;
};

layout(set = 0, binding = 0) uniform sampler Samplers[];
layout(set = 0, binding = 1) uniform texture2D Textures[];

layout(push_constant, std430) uniform args {
  SpriteDataBuffer sprite_data;
};

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 color_attachment;

void main() {
  uint tex_idx = nonuniformEXT(sprite_data.texture_index);
  uint smp_idx = nonuniformEXT(0);
  
  vec4 tex_color = texture(sampler2D(Textures[tex_idx], Samplers[smp_idx]), uv);
  color_attachment = tex_color * sprite_data.tint_color;
}
