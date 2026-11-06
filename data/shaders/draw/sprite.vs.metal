using namespace metal;

constant float4 vertecies[] = {
  float4(0.0f, 1.0f, 0.0f, 1.0f),
  float4(1.0f, 1.0f, 0.0f, 1.0f),
  float4(1.0f, 0.0f, 0.0f, 1.0f),
  float4(0.0f, 0.0f, 0.0f, 1.0f),
};

constant float2 uv[] = {
  float2(0.0f, 1.0f),
  float2(1.0f, 1.0f),
  float2(1.0f, 0.0f),
  float2(0.0f, 0.0f),
};

constant uint indecies[] = {
  0, 1, 3,
  1, 2, 3
};

struct SpriteData {
  float4x4         projection;
  float4x4         transform;
  float4           part_position_size;
  float2           texture_size;
  float4           tint_color;
  texture2d<float> texture;
  sampler          sampler;
};

struct VertexOutput {
  float4 position [[position]];
  float2 uv;
};

vertex VertexOutput VertexMain(
  uint vertex_index [[vertex_id]],
  constant SpriteData* sprite_data [[buffer(1)]]
) {
  VertexOutput output = {0};

  float4 v = sprite_data->transform*vertecies[indecies[vertex_index]];
  float2 local_uv = uv[indecies[vertex_index]];
  output.position = sprite_data->projection*v;
  output.uv = (sprite_data->part_position_size.xy + local_uv*sprite_data->part_position_size.zw)/sprite_data->texture_size;
  return output;
}
