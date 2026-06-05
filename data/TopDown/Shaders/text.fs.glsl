#version 460

#extension GL_EXT_buffer_reference    : require
#extension GL_EXT_buffer_reference2   : require

layout(buffer_reference) readonly buffer GlyphDataBuffer {
  mat4x4 projection;
  vec4   position_size;
  vec3   color;
  int    texture_index;// --AlNov: @TODO Texture
};

layout(push_constant, std430) uniform args {
  GlyphDataBuffer glyph_data;
};

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 color_attachment;

void main() {
  color_attachment = vec4(glyph_data.color, 1.0);
}
