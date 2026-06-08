#version 460
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_buffer_reference     : require
#extension GL_EXT_buffer_reference2    : require

layout(buffer_reference) readonly buffer GlyphDataBuffer {
  mat4x4 projection;
  vec4   position_size;
  vec4   color;
  uint   texture_index;// --AlNov: @TODO Texture
};

layout(set = 0, binding = 0) uniform sampler   Samplers[];
layout(set = 0, binding = 1) uniform texture2D Textures[];

layout(push_constant, std430) uniform args {
  GlyphDataBuffer glyph_data;
};

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 color_attachment;

void main() {
  float alpha = texture(sampler2D(Textures[nonuniformEXT(glyph_data.texture_index)], Samplers[nonuniformEXT(0)]), uv).r;
  color_attachment = vec4(glyph_data.color.rgb, glyph_data.color.a*alpha);
}
