using namespace metal;

struct VertexOutput {
  float4 position [[position]];
  float2 uv;
};

struct SpriteData {
  float4x4         projection;
  float4           position_size;
  float4           part_position_size;
  float4           tint_color;
  texture2d<float> texture;
};

fragment float4 FragmentMain(
  VertexOutput input [[stage_in]],
  constant SpriteData* sprite_data [[buffer(1)]]
) {
  constexpr sampler sprite_sampler(address::clamp_to_edge, filter::nearest);
  return sprite_data->texture.sample(sprite_sampler, input.uv);
}
