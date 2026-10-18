#version 460

#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_buffer_reference     : require
#extension GL_EXT_buffer_reference2    : require

layout(buffer_reference) readonly buffer SpriteDataBuffer {
  mat4x4 projection;
  mat4x4 transform;
  vec4   part_position_size;
  vec2   texture_size;
  vec4   tint_color;
  uint   texture_index;// --AlNov: @TODO Texture
};

layout(set = 0, binding = 0) uniform sampler   Samplers[];
layout(set = 0, binding = 1) uniform texture2D Textures[];

layout(push_constant, std430) uniform args {
  SpriteDataBuffer sprite_data;
};

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 color_attachment;

void main() {
  color_attachment = texture(sampler2D(Textures[nonuniformEXT(sprite_data.texture_index)], Samplers[nonuniformEXT(0)]), uv);
}
