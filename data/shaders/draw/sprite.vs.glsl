#version 460

#extension GL_EXT_nonuniform_qualifier          : require
#extension GL_EXT_buffer_reference              : require
#extension GL_EXT_buffer_reference2             : require
#extension GL_EXT_samplerless_texture_functions : require

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

layout(buffer_reference) readonly buffer SpriteDataBuffer {
  mat4x4 projection;
  mat4x4 transform;
  vec4   part_position_size;
  vec2   texture_size;
  vec4   tint_color;
  uint   texture_index;// --AlNov: @TODO Texture
};

layout(push_constant, std430) uniform args {
  SpriteDataBuffer sprite_data;
};

layout(location = 0) out vec2 out_uv;

  void main() {
  vec4 vertex = sprite_data.transform*vertecies[indecies[gl_VertexIndex]];
  vec2 local_uv = uv[indecies[gl_VertexIndex]];
  out_uv = (sprite_data.part_position_size.xy + local_uv*sprite_data.part_position_size.zw)/sprite_data.texture_size;
  gl_Position = sprite_data.projection*vertex;
}
