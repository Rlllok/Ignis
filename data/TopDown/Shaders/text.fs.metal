using namespace metal;

struct VertexOutput {
  float4 position [[position]];
  float2 uv;
  uint   instance_index;
};

struct GlyphData {
  float4x4         projection;
  float4           position_size;
  float3           color;
  texture2d<float> texture;
};

fragment float4 FragmentMain(
  VertexOutput input [[stage_in]],
  constant GlyphData* glyph_data [[buffer(1)]]
) {
  GlyphData glyph = glyph_data[input.instance_index];

  constexpr sampler glyph_sampler(address::clamp_to_edge, filter::linear);

  return float4(glyph.color, glyph.texture.sample(glyph_sampler, input.uv).r);
}
