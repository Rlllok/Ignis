#version 460

#extension GL_EXT_nonuniform_qualifier           : require
#extension GL_EXT_buffer_reference              : require
#extension GL_EXT_buffer_reference2             : require
#extension GL_EXT_samplerless_texture_functions : require

// Standard 6-vertex quad (no index array needed with vkCmdDraw 6 vertices)
const vec4 positions[6] = vec4[](
  vec4(0.0, 1.0, 0.0, 1.0),
  vec4(1.0, 1.0, 0.0, 1.0),
  vec4(0.0, 0.0, 0.0, 1.0),
  vec4(1.0, 1.0, 0.0, 1.0),
  vec4(1.0, 0.0, 0.0, 1.0),
  vec4(0.0, 0.0, 0.0, 1.0)
);

const vec2 uvs[6] = vec2[](
  vec2(0.0, 1.0),
  vec2(1.0, 1.0),
  vec2(0.0, 0.0),
  vec2(1.0, 1.0),
  vec2(1.0, 0.0),
  vec2(0.0, 0.0)
);

layout(set = 0, binding = 1) uniform texture2D Textures[];

layout(buffer_reference, std430) readonly buffer SpriteDataBuffer {
  mat4x4 projection;
  mat4x4 transform;
  vec4   part_position_size;
  vec4   tint_color;
  uint   texture_index;
  uint   sampler_index;
};

layout(push_constant, std430) uniform args {
  SpriteDataBuffer sprite_data;
};

layout(location = 0) out vec2 out_uv;

void main() {
  ivec2 texture_size = textureSize(Textures[nonuniformEXT(sprite_data.texture_index)], 0);
  
  gl_Position = sprite_data.projection * sprite_data.transform * positions[gl_VertexIndex];
  
  vec2 base_uv = uvs[gl_VertexIndex];
  vec2 sprite_size = vec2(texture_size);
  
  // Guard against divide-by-zero if texture isn't loaded yet
  if (sprite_size.x > 0.0 && sprite_size.y > 0.0) {
    out_uv = (sprite_data.part_position_size.xy + (base_uv * sprite_data.part_position_size.zw)) / sprite_size;
  } else {
    out_uv = base_uv;
  }
}
