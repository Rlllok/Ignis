#version 460

#extension GL_EXT_buffer_reference    : require
#extension GL_EXT_buffer_reference2   : require

const vec4 vertecies[] = {
  vec4(0.0f, 1.0f, 0.0f, 1.0f),
  vec4(1.0f, 1.0f, 0.0f, 1.0f),
  vec4(1.0f, 0.0f, 0.0f, 1.0f),
  vec4(0.0f, 0.0f, 0.0f, 1.0f),
};

const vec2 uv[] = {
  vec2(0.0f, 1.0f),
  vec2(1.0f, 1.0f),
  vec2(1.0f, 0.0f),
  vec2(0.0f, 0.0f),
};

const uint indecies[] = {
  0, 1, 3,
  1, 2, 3
};

layout(buffer_reference) readonly buffer GlyphDataBuffer {
  mat4x4 projection;
  vec4   position_size;
  vec3   color;
  int    texture_index;// --AlNov: @TODO Texture
};

layout(push_constant, std430) uniform args {
  GlyphDataBuffer glyph_data;
};

layout(location = 0) out vec2 out_uv;

void main() {
  vec4 offset = vec4(glyph_data.position_size.xy, 0.0f, 0.0f);
  vec4 scale = vec4(glyph_data.position_size.zw, 1.0f, 1.0f);
  vec4 vertex = vertecies[indecies[gl_VertexIndex]]*scale + offset;
  out_uv = uv[indecies[gl_VertexIndex]];
  gl_Position = glyph_data.projection*vertex;
}
